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

#include <curl/curl.h>

#define MAX_IFS 64

// #define LOG_DEFAULT_LEVEL 2

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
				return ip;
			}
		}
	}

	return "";
}
