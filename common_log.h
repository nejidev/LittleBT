#ifndef __COMMON__LOG__H_
#define __COMMON__LOG__H_

#include <stdio.h>

#define LOG_TRACE_LEVEL 0
#define LOG_DEBUG_LEVEL 1
#define LOG_INFO_LEVEL  2
#define LOG_WARN_LEVEL  3
#define LOG_ERROR_LEVEL 4
#define LOG_FATAL_LEVEL 5

#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL LOG_TRACE_LEVEL
#endif

#define LOG_PRINTF(fmt, ...) if ( LOG_DEFAULT_LEVEL <= LOG_DEBUG_LEVEL ) printf(fmt, ##__VA_ARGS__);
#define LOG_DEBUG(fmt, ...) if ( LOG_DEFAULT_LEVEL <= LOG_DEBUG_LEVEL ) printf("[Debug %s %d]" fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__);
#define LOG_INFO(fmt, ...)  if ( LOG_DEFAULT_LEVEL <= LOG_INFO_LEVEL ) printf("[Info %s %d]" fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__);

#endif // __COMMON__LOG__H_
