/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_SBRK_PAGE_ALLOCATOR_H__
#define __FT_SBRK_PAGE_ALLOCATOR_H__

#include "ft_sys_alloc_intf.h"

namespace ftmalloc
{
    class CSbrkPageAllocator : public ISysAlloc
    {
    public:
        CSbrkPageAllocator();
        virtual ~CSbrkPageAllocator();

        virtual void * SysAlloc(size_t size);
        virtual void SysRelease(void * ptr, size_t size);

    private:
        bool m_bAlign;
    };
}
#endif