/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_sbrk_page_allocator.h"
#include "ft_malloc_util.h"
#include "ft_malloc_log.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>

namespace ftmalloc
{
    CSbrkPageAllocator s_sbrk_page_allocator;
    
    CSbrkPageAllocator::CSbrkPageAllocator()
        : m_bAlign(false)
    {
    }
    
    CSbrkPageAllocator::~CSbrkPageAllocator()
    {
    }
    
    void * CSbrkPageAllocator::SysAlloc(size_t size)
    {
        if (!m_bAlign) {    
            void * paddr = sbrk(0);
            size_t addr = (size_t)paddr;
            FT_LOG(FT_INFO, "algin, addr:%p", paddr);
            
            if (addr & ((1 << FT_PAGE_BIT) - 1)) {
                addr = addr + (1 << FT_PAGE_BIT);
                addr = addr & (~((1 << FT_PAGE_BIT) - 1));

                size_t align_size = addr - (size_t)paddr;
                sbrk(align_size);
                FT_LOG(FT_INFO, "after algin, addr:%p, alignsize:%zd, newaddr:%p", paddr, align_size, sbrk(0));
            }
            m_bAlign = true;
        }

        void * addr = sbrk(size);
        if (addr == NULL) {
            FT_LOG(FT_ERROR, "sbrk failed, errno:%d, %s", errno, strerror(errno));
        }
        FT_LOG(FT_DEBUG, "sbrk, size:%zd, addr:%p", size, addr);

        return addr;
    }
    
    void CSbrkPageAllocator::SysRelease(void * ptr, size_t size)
    {
        FT_LOG(FT_DEBUG, "brk size:%zd, addr:%p", size, ptr);
        if (brk(ptr)) {
            FT_LOG(FT_ERROR, "brk failed, errno:%d, %s", errno, strerror(errno));
        }
    }
}