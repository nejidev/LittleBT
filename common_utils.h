#ifndef __COMMON_UTILS__H_
#define __COMMON_UTILS__H_

#include <string>

std::string urlEncode(const char *buff, int length);
std::string getLocalIP();
void fileWriteBin(const char *path, const char *buff, int length);
int fileReadBin(const char *path, char *buff, int length);
std::string inetNtoaString(const unsigned char *buff);
std::string getLocalPeerId();

#endif // __COMMON_UTILS__H_
