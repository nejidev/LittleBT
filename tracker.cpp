#include "tracker.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

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

Tracker::Tracker(std::string announce, std::string infoHash):thread_runing{false}
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

	LOG_DEBUG("callback size:%ld nmemb:%ld ptr:%s", size, nmemb, (char*)ptr);

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
		}
		else if ( memory_data )
		{
			LOG_DEBUG("position:%ld %s", memory_data->position, memory_data->memory);
			memory_data->position = 0;
		}

		std::this_thread::sleep_for(std::chrono::seconds(10));
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

	auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::stringstream time_fmt;
	char peer_id[20] = { 0 };
	std::string param = "?info_hash=" + infoHash;

	time_fmt << std::put_time(std::localtime(&time), "%Y-%m-%d");

	snprintf(peer_id, sizeof(peer_id), "LittleBT%s", time_fmt.str().c_str());

	LOG_DEBUG("peer_id:%s", peer_id);

	param += "&peer_id=" + urlEncode(peer_id, sizeof(peer_id));

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

void Tracker::start()
{
	if ( 0 != announce.find("http") )
	{
		LOG_DEBUG("not support protocol:%s", announce.c_str());
		return ;
	}

	startThread();
}

void Tracker::stop()
{
	stopThread();
}

void Tracker::complete()
{
	stopThread();
}
