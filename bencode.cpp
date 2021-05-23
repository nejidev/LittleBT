#include "bencode.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/sha.h>
#include <curl/curl.h>

#include "common_utils.h"

// #define LOG_DEFAULT_LEVEL 2

#include "common_log.h"

Bencode::Bencode():raw_buffer{NULL}, raw_buffer_len{0},is_files{false}
{

}

Bencode::~Bencode()
{
	if ( raw_buffer )
	{
		free(raw_buffer);
		raw_buffer = NULL;
	}

	announce_list.clear();
	file_entity_list.clear();
}

void Bencode::setRawBuffer(char *buff, int len)
{
	if ( buff && 0 < len )
	{
		raw_buffer = (char *)malloc(len);

		if ( ! raw_buffer )
		{
			return ;
		}

		memset(raw_buffer, 0, len);
		memcpy(raw_buffer, buff, len);
		raw_buffer_len = len;
	}
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

void Bencode::sha1Encode(const char *buff, int len)
{
	int i = 0;
	char format_buffer[8] = { 0 };

	LOG_DEBUG("buff len:%d hex dump", len);

	// for ( i=0; i<=len; i++ )
	// {
	// 	LOG_PRINTF("%02X ", (unsigned char)buff[i]);
	// }

	SHA1((unsigned char *)buff, len, (unsigned char *)info_hash_digest);

	for ( i=0; i<sizeof(info_hash_digest); i++ )
	{
		snprintf(format_buffer, sizeof(format_buffer), "%02X", (unsigned char)info_hash_digest[i]);
		info_hash.append(format_buffer);
	}

	LOG_DEBUG("\r\nsha1 dump: %s", info_hash.c_str());

	info_hash_url_encode = urlEncode((const char *)info_hash_digest, sizeof(info_hash_digest));

	LOG_DEBUG("sha1 encode:%s", info_hash_url_encode.c_str());
}

void Bencode::decode()
{
	int i = 0;
	int pos = 0;
	int int_val = 0;
	char *str_val = NULL;
	int len = raw_buffer_len;
	const char *buff = raw_buffer;
	bool capture_path = false;
	FileEntity file_entity;

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
				if ( 0 == strcmp("creation date", str_val) )
				{
					creation_time = int_val;
					int_val = 0;
					LOG_DEBUG("creation_time:%d", creation_time);
				}
				else if ( 0 == strcmp("length", str_val) )
				{
					length = int_val;
					int_val = 0;

					file_entity.setLength(length);

					LOG_DEBUG("length:%d", length);
				}
				else if ( 0 == strcmp("piece length", str_val) )
				{
					piece_length = int_val;
					int_val = 0;
					LOG_DEBUG("piece_length:%d", piece_length);
				}
				else if( 0 == strcmp("path", str_val) || 0 == strcmp("name", str_val) )
				{
					capture_path = true;
				}

				free(str_val);
				str_val = NULL;
			}

			if ( 0 < int_val )
			{
				str_val = (char *)malloc(int_val + 1);
				memset(str_val, 0 , int_val + 1);
				memcpy(str_val, buff + i + pos +1, int_val);

				if ( strstr(str_val, "announce") &&  int_val > strlen("announce") )
				{
					announce_list.push_back(str_val);
				}

				if ( capture_path )
				{
					capture_path = false;

					file_entity.setPath(str_val);
					file_entity_list.push_back(file_entity);

					LOG_DEBUG("capture path:%s", str_val);
				}

				LOG_DEBUG("str_val:%s len:%ld", str_val, strlen(str_val));
			}
		}

		i += pos;
		i += int_val;

		if ( str_val )
		{
			if ( 0 == strcmp("info", str_val) )
			{
				info_hash_start = i + 1;
				info_hash_end = len - i - 1 - 1;
			}
			else if ( 0 == strcmp("nodes", str_val) )
			{
				info_hash_end = i - info_hash_start - strlen("nodes") - 3;
			}

			else if ( strstr(str_val, "files") )
			{
				is_files = true;
			}
		}

		int_val = 0;
		pos = 0;
	}

	if ( is_files && ! file_entity_list.empty() )
	{
		file_entity = file_entity_list.back();
		file_entity_list.pop_back();
		files_path = file_entity.getPath();

		LOG_DEBUG("files_path:%s", files_path.c_str());
	}

	sha1Encode(buff + info_hash_start, info_hash_end);
}

void Bencode::showFileEntity()
{
	int file_length = 0;
	std::string file_path = "";

	for ( auto &file : file_entity_list )
	{
		file_length = file.getLength();
		file_path   = file.getPath();

		//filter _____padding_file_0_如果您看到此文件，请升级到BitComet(比特彗星)0.85或以上版本____

		if ( std::string::npos == file_path.find("_____padding_file_" ) )
		{
			LOG_INFO("file length:%d %d.%dMB path:%s", file_length, file_length/1024/1024,
				(file_length - (file_length/1024/1024 * 1024 * 1024))*1000/1024/1024/10,
				file_path.c_str());
		}
	}
}

std::vector<std::string> Bencode::getAnnounceList() const
{
	return announce_list;
}

std::string Bencode::getInfoHash() const
{
	return info_hash_url_encode;
}
