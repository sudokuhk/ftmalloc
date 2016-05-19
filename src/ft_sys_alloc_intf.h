/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_SYS_ALLOC_INTF_H__
#define __FT_SYS_ALLOC_INTF_H__

#include <stddef.h>

namespace ftmalloc
{
    class ISysAlloc
    {
    public:
        virtual ~ISysAlloc() {}

        virtual void * SysAlloc(size_t size) = 0;
        virtual void SysRelease(void * ptr, size_t size) = 0;
    };
}

#endif
