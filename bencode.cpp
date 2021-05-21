#include "bencode.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/sha.h>
#include <curl/curl.h>

#include "common_log.h"

Bencode::Bencode():raw{""}
{

}

Bencode::Bencode(std::string &str):raw{str}
{

}

void Bencode::setRaw(std::string &str)
{
	raw = str;
}

static int findInt(const char *buff, int len, int *out_int)
{
	int i = 0;
	int pos = 0;

	for ( i=0; i<len; i++ )
	{
		//if ( ':' == buff[i] )
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

static std::string sha1Encode(const char *buff, int len)
{
	int i = 0;
	std::string sha1;
	char *sha1_url_encode = NULL;
	unsigned char digest[SHA_DIGEST_LENGTH] = { 0 };

	SHA1((unsigned char *)buff, len, (unsigned char *)&digest);

	LOG_DEBUG("sha1 dump:");

	for ( i=0; i<sizeof(digest); i++ )
	{
		printf("%02X ", digest[i]);
	}

	sha1_url_encode = curl_escape((const char *)digest, sizeof(digest));

	if ( sha1_url_encode )
	{
		sha1 = sha1_url_encode;
		LOG_DEBUG("sha1 encode:%s", sha1_url_encode);
	}

	curl_free(sha1_url_encode);
	sha1_url_encode = NULL;

	return sha1;
}

void Bencode::decode()
{
	int i = 0;
	int pos = 0;
	int int_val = 0;
	char *str_val = NULL;
	int len = raw.length();
	const char *buff = raw.c_str();

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
					LOG_DEBUG("length:%d", length);
				}
				else if ( 0 == strcmp("piece length", str_val) )
				{
					piece_length = int_val;
					int_val = 0;
					LOG_DEBUG("piece_length:%d", piece_length);
				}

				free(str_val);
				str_val = NULL;
			}

			if ( 0 < int_val )
			{
				str_val = (char *)malloc(int_val + 1);
				memset(str_val, 0 , int_val + 1);
				memcpy(str_val, buff + i + pos +1, int_val);

				if ( strstr(str_val, "announce") )
				{
					announce_list.push_back(str_val);
				}

				LOG_DEBUG("str_val:%s len:%ld", str_val, strlen(str_val));
			}
		}

		i += pos;
		i += int_val;

		int_val = 0;
		pos = 0;
	}

	sha1Encode("1234", 4);
}

Bencode::~Bencode()
{
	announce_list.clear();
}
