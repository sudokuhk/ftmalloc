/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_MEM_ALLOC_INTF_H__
#define __FT_MEM_ALLOC_INTF_H__

#include <stddef.h>

namespace ftmalloc
{
    class IMemAlloc
    {
    public:
        virtual ~IMemAlloc() {}

        virtual void * Malloc(size_t) = 0;
        virtual void * ReAlloc(void *, size_t) = 0;
        virtual void * Calloc(size_t, size_t) = 0;
        virtual void Free(void *) = 0;

        static IMemAlloc * CreateMemAllocator();
        static void DestroyMemAllocator(IMemAlloc * &allocator);
    };
}

#endif
