/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_MALLOC_LOG_H__
#define __FT_MALLOC_LOG_H__

#include <stdio.h>
#include <pthread.h>

#define FT_EMERG    1
#define FT_ALERT    2
#define FT_CRIT     3
#define FT_ERROR    4
#define FT_NOTICE   5
#define FT_INFO     6
#define FT_DEBUG    7

extern const char * s_ft_log_level_string[];
extern int s_ft_log_level;

#ifdef LOG_PRINTF
#define FT_LOG(level, fmt, ...)                                     \
    do {                                                            \
        if (level <= s_ft_log_level) {                              \
            printf("%30s%20s:%5d:[TID:%lu][%s]:"fmt"\n",            \
            __FILE__, __FUNCTION__, __LINE__, pthread_self(),       \
            s_ft_log_level_string[level], ##__VA_ARGS__);           \
        }                                                           \
    } while (0)
#else
#define FT_LOG(level, fmt, ...)
#endif

#endif
