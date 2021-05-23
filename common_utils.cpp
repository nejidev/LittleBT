#include "common_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h> 
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>

#include <curl/curl.h>

#define MAX_IFS 64

#define LOG_DEFAULT_LEVEL 2

#include "common_log.h"

std::string urlEncode(const char *buff, int length)
{
	std::string url_encode;
	char *result = NULL;

	result = curl_escape(buff, length);

	if ( result )
	{
		url_encode = result;
	}

	LOG_DEBUG("sha1 encode:%s", result);

	curl_free(result);
	result = NULL;

	return url_encode;
}

std::string getLocalIP()
{
	std::string ip = "";

	struct ifreq *ifr  = NULL, *ifend = NULL;
	struct ifreq ifreq = { 0 };
	struct ifconf ifc  = { 0 };
	struct ifreq ifs[MAX_IFS] = { 0 };
	int sockfd = -1;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	ifc.ifc_len = sizeof(ifs);
	ifc.ifc_req = ifs;

	if ( 0 > ioctl(sockfd, SIOCGIFCONF, &ifc) )
	{
		LOG_ERROR("ioctl(SIOCGIFCONF) failed");
		return "";
	}

	ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));

	for ( ifr = ifc.ifc_req; ifr < ifend; ifr++ )
	{
		if ( AF_INET == ifr->ifr_addr.sa_family )
		{
			strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));

			if ( 0 > ioctl(sockfd, SIOCGIFHWADDR, &ifreq) )
			{
				LOG_ERROR("SIOCGIFHWADDR(%s): failed", ifreq.ifr_name);
				close(sockfd);
				return "";
			}

			ip = inet_ntoa( ( (struct sockaddr_in *)  &ifr->ifr_addr)->sin_addr);

			LOG_DEBUG("\nDevice %s -> Ethernet %s %02x:%02x:%02x:%02x:%02x:%02x\t",
					ifreq.ifr_name, ip.c_str(),
					(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[0],
					(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[1],
					(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[2],
					(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[3],
					(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[4],
					(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[5]);

			if ( "127.0.0.1" != ip )
			{
				close(sockfd);
				return ip;
			}
		}
	}

	close(sockfd);
	return "";
}

void fileWriteBin(const char *path, const char *buff, int length)
{
	std::ofstream file_stream(path, std::ios::out | std::ios::binary | std::ios::trunc);

	file_stream.write(buff, length);
	file_stream.close();
}

int fileReadBin(const char *path, char *buff, int length)
{
	int file_size = 0;
	std::ifstream file_stream(path, std::ios::in | std::ios::binary);

	file_stream.seekg(0, std::ios::end);
	file_size = file_stream.tellg();
	file_stream.seekg(0, std::ios::beg);

	file_stream.read(buff, std::min(file_size, length));

	file_stream.close();

	return std::min(file_size, length);
}

int fileGetSize(const char *path)
{
	int file_size = 0;
	std::ifstream file_stream(path, std::ios::in | std::ios::binary);

	file_stream.seekg(0, std::ios::end);
	file_size = file_stream.tellg();
	file_stream.seekg(0, std::ios::beg);
	file_stream.close();

	return file_size;
}

std::string inetNtoaString(const unsigned char *buff)
{
	struct in_addr addr = { 0 };

	if ( ! buff )
	{
		return "";
	}

	memcpy(&addr, buff, sizeof(addr));
	return inet_ntoa(addr);
}

std::string getLocalPeerId()
{
	static std::string peer_id = "";

	if ( peer_id.empty() )
	{
		static std::default_random_engine e;
		static std::uniform_int_distribution<unsigned> u(0, 9);

		auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::stringstream time_fmt;

		e.seed(time);

		time_fmt << std::put_time(std::localtime(&time), "LBT%Y%m%d%H%M%S");

		time_fmt << u(e);
		time_fmt << u(e);
		time_fmt << u(e);

		peer_id = time_fmt.str();

		LOG_DEBUG("peer_id:%s", peer_id.c_str());

	}

	return peer_id;
}
