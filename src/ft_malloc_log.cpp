/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#include "ft_malloc_log.h"

#if defined(FT_LOG_EMRGE)
int s_ft_log_level = FT_EMERG;
#elif defined(FT_LOG_ALERT)
int s_ft_log_level = FT_ALERT;
#elif defined(FT_LOG_CRIT)
int s_ft_log_level = FT_CRIT;
#elif defined(FT_LOG_ERROR)
int s_ft_log_level = FT_ERROR;
#elif defined(FT_LOG_NOTICE)
int s_ft_log_level = FT_NOTICE;
#elif defined(FT_LOG_INFO)
int s_ft_log_level = FT_INFO;
#elif defined(FT_LOG_DEBUG)
int s_ft_log_level = FT_DEBUG;
#else
int s_ft_log_level = FT_INFO;
#endif

const char * s_ft_log_level_string[] =
{
    "",
    "EMERG",
    "ALERT",
    "CRIT",
    "ERROR",
    "NOTICE",
    "INFO",
    "DEBUG",
};

