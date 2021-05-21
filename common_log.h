#ifndef __COMMON__LOG__H_
#define __COMMON__LOG__H_

#include <stdio.h>

#define LOG_DEBUG(fmt, ...) printf("[D]" fmt "\r\n", ##__VA_ARGS__);
#define LOG_INFO(fmt, ...) printf("[I]" fmt "\r\n", ##__VA_ARGS__);

#endif // __COMMON__LOG__H_
