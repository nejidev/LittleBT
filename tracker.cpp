#include "tracker.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <curl/curl.h>

#include "common_utils.h"

// #define LOG_DEFAULT_LEVEL 2

#include "common_log.h"

#define MEMORY_CHUNK_SIZE (1024*16)

typedef struct MemoryData
{
	char *memory;
	size_t position;
	size_t capacity;
} MemoryData;

Tracker::Tracker(std::string announce, std::string infoHash):thread_runing{false},
complete{0}, incomplete{0}, interval{0}, min_interval{0}, peers{0}
{
	this->announce = announce;
	this->infoHash = infoHash;
}

Tracker::~Tracker()
{
}

static size_t sizeAlign(size_t size, size_t align)
{
	align--;
	return (size + align) & ~align;
}

static MemoryData *memoryDataBuffer(char *buffer, size_t size)
{
	MemoryData *mem  = NULL;

	mem = (MemoryData *)malloc(sizeof(*mem));
	if ( ! mem )
	{
		LOG_ERROR("malloc failed");
		return NULL;
	}

	memset(mem, 0, sizeof(*mem));
	if ( ! buffer && 0 == size )
	{
		size   = MEMORY_CHUNK_SIZE;
		buffer = (char *)malloc(size);
		if ( ! buffer )
		{
			LOG_ERROR("malloc failed");
			return NULL;
		}
	}
	mem->memory   = buffer;
	mem->capacity = size;

	return mem;
}

static size_t writeDataCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	MemoryData  *mem = (MemoryData *)data;
	size_t realsize  = size * nmemb;
	size_t newsize   = mem->position + realsize;
	int write_pos    = 0;
	int ret          = 0;

	//LOG_DEBUG("callback size:%ld nmemb:%ld ptr:%s", size, nmemb, (char*)ptr);

	//auto memory pump
	if ( newsize > mem->capacity )
	{
		newsize     = sizeAlign(newsize, MEMORY_CHUNK_SIZE);
		mem->memory = (char *)realloc(mem->memory, newsize);

		if ( ! mem->memory )
		{
			LOG_ERROR("malloc failed");
			return 0;
		}
		mem->capacity = newsize;
	}

	if ( mem->memory )
	{
		memcpy(mem->memory + mem->position, ptr, realsize);
		mem->position += realsize;

		return realsize;
	}

	return 0;
}

static int findInt(const char *buff, int len, int *out_int)
{
	int i = 0;
	int pos = 0;

	for ( i=0; i<len; i++ )
	{
		if ( '0' > buff[i] || '9' < buff[i] )
		{
			break;
		}
		pos++;
	}

	*out_int = strtoul(buff, NULL, 0);

	LOG_DEBUG("out_int:%d", *out_int);

	return pos;
}

void Tracker::benDecode(const char *buff, int len)
{
	int i = 0;
	int pos = 0;
	int int_val = 0;
	char *str_val = NULL;

	for ( i=0; i<len; i++ )
	{
		if ( 'd' == buff[i] )
		{
			pos = findInt(buff + i + 1, len - i - 1, &int_val);
			i++;
		}
		else if ( '0' <= buff[i] && '9' >= buff[i] )
		{
			pos = findInt(buff + i, len - i, &int_val);
		}

		if ( 0 < int_val )
		{
			if ( str_val )
			{
				if ( 0 == strcmp("complete", str_val) )
				{
					complete = int_val;
					int_val = 0;
					LOG_DEBUG("complete:%d", complete);
				}
				else if ( 0 == strcmp("incomplete", str_val) )
				{
					incomplete = int_val;
					int_val = 0;
					LOG_DEBUG("incomplete:%d", incomplete);
				}
				else if ( 0 == strcmp("downloaded", str_val) )
				{
					downloaded = int_val;
					int_val = 0;
					LOG_DEBUG("downloaded:%d", downloaded);
				}
				else if ( 0 == strcmp("interval", str_val) )
				{
					interval = int_val;
					int_val = 0;
					LOG_DEBUG("interval:%d", interval);
				}
				else if ( 0 == strcmp("min interval", str_val) )
				{
					min_interval = int_val;
					int_val = 0;
					LOG_DEBUG("min_interval:%d", min_interval);
				}
				else if ( 0 == strcmp("peers", str_val) )
				{
					peers = int_val;
					int_val = 0;

					LOG_DEBUG("peers:%d", peers);

					peersDecode(strstr(buff + i, ":") + 1, peers);
				}

				free(str_val);
				str_val = NULL;
			}

			if ( 0 < int_val )
			{
				str_val = (char *)malloc(int_val + 1);
				memset(str_val, 0 , int_val + 1);
				memcpy(str_val, buff + i + pos +1, int_val);

				LOG_DEBUG("str_val:%s len:%ld", str_val, strlen(str_val));
			}
		}

		i += pos;
		i += int_val;

		int_val = 0;
		pos = 0;
	}
}

void Tracker::peersDecode(const char *buff, int len)
{
	int i = 0;
	std::string ip = "";
	int port = 0;
	peer_t *peer = (peer_t *)buff;

	LOG_DEBUG("buff:%s len:%d", buff, len);

	i = len / sizeof(*peer);

	while ( i-- )
	{
		if ( peer && peer->ip && peer->port )
		{
			ip = inetNtoaString(peer->ip);
			memcpy(&port, peer->port, sizeof(peer->port));
			port = ntohs(port);

			LOG_DEBUG("peers ip:%s port:%d", ip.c_str(), port);
		}

		peer++;
	}
}

void Tracker::threadRun()
{
	std::string url = announce;
	CURL *curl = NULL;
	CURLcode code = CURLE_OK;
	struct curl_slist *headers = NULL;
	MemoryData *memory_data = NULL;

	url += buildUrlParam();

	curl = curl_easy_init();

	if ( ! curl )
	{
		LOG_ERROR("curl init failed");
		return ;
	}

	LOG_DEBUG("start Run url:%s", url.c_str());

	memory_data = memoryDataBuffer(NULL, 0);

	headers = curl_slist_append(headers, "User-Agent: Bittorrent");

	curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL,       1);
	curl_easy_setopt(curl, CURLOPT_HEADER,         0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 120);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT,        120);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)memory_data);

	while ( thread_runing )
	{
		code = curl_easy_perform(curl);

		if ( CURLE_OK != code )
		{
			LOG_ERROR("curl failed code:%d msg:%s", code, curl_easy_strerror(code));
			thread_runing = false;
		}
		else if ( memory_data )
		{
			LOG_DEBUG("url:%s length:%ld", url.c_str(), memory_data->position);

			if ( 0 < memory_data->position )
			{
				benDecode(memory_data->memory, memory_data->position);
				memory_data->position = 0;
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(interval));
	}

	//cleanup
	if ( headers )
	{
		curl_slist_free_all(headers);
		headers = NULL;
	}

	if ( curl )
	{
		curl_easy_cleanup(curl);
		curl = NULL;
	}

	if ( memory_data )
	{
		if ( memory_data->memory )
		{
			free(memory_data->memory);
			memory_data->memory = NULL;
		}

		free(memory_data);
		memory_data = NULL;
	}
}

std::string Tracker::buildUrlParam()
{
	/**
	 * example
	 * announce?info_hash=%87%2F%DF%A8K%97%B5%EC%7B%A6h%CA%B5%FF%28%40%FEK%22%0F&peer_id=%2DSD0100%2DUz%A8%F8%11L%8AB%29%1C%DA%25&ip=172.16.5.140&port=9565&uploaded=17712&downloaded=0&left=378440308&numwant=200&key=18360&compact=1&event=started
	 */

	std::string peer_id = getLocalPeerId();
	std::string param = "?info_hash=" + infoHash;

	param += "&peer_id=" + urlEncode(peer_id.c_str(), peer_id.length());

	param += "&ip=" + getLocalIP();
	param += "&port=9565&uploaded=17712&downloaded=0&left=378440308&numwant=200&key=18360&compact=1&event=started";

	return param;
}

void Tracker::startThread()
{
	thread_runing = true;
	thread_tid = std::thread(&Tracker::threadRun, this);
}

void Tracker::stopThread()
{
	thread_runing = false;

	thread_tid.join();
}

void Tracker::startEvent()
{
	if ( 0 != announce.find("http") )
	{
		LOG_DEBUG("not support protocol:%s", announce.c_str());
		return ;
	}

	startThread();
}

void Tracker::stopEvent()
{
	stopThread();
}

void Tracker::completeEvent()
{
	stopThread();
}
