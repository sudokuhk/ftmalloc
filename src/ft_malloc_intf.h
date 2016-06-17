/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_MALLOC_INTF_H__
#define __FT_MALLOC_INTF_H__

#include <stddef.h>

namespace ftmalloc
{    
    void * __Malloc(size_t);

    void * __ReAlloc(void *, size_t);

    void * __Calloc(size_t, size_t);

    void __Free(void *);
}

#endif