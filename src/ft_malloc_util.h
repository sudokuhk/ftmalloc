/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_MALLOC_UTIL_H__
#define __FT_MALLOC_UTIL_H__

#define SAFE_FREE(ptr)          \
    do {                        \
        if (ptr != NULL) {      \
            free(ptr);          \
            ptr = NULL;         \
        }                       \
    } while (0)
    
#define SAFE_DELETE(ptr)        \
    do {                        \
        if (ptr != NULL) {      \
            delete (ptr);       \
            ptr = NULL;         \
        }                       \
    } while (0)

#define FT_PAGE_BIT 16

#define FT_MAX(a, b)    ((a) > (b) ? (a) : (b))
#define FT_MIN(a, b)    ((a) < (b) ? (a) : (b))

#endif
