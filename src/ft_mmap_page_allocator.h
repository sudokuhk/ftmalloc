/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_MMAP_PAGE_ALLOCATOR_H__
#define __FT_MMAP_PAGE_ALLOCATOR_H__

#include "ft_sys_alloc_intf.h"

namespace ftmalloc
{
    class CMmapPageAllocator : public ISysAlloc
    {
    public:
        CMmapPageAllocator();
        virtual ~CMmapPageAllocator();

        virtual void * SysAlloc(size_t size);
        virtual void SysRelease(void * ptr, size_t size);
    };
}

#endif