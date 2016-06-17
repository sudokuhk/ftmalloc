/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_mmap_page_allocator.h"
#include "ft_malloc_log.h"

#include <string.h>
#include <errno.h>

#include <sys/mman.h>

namespace ftmalloc
{
    CMmapPageAllocator s_mmap_page_allocator;
    
    CMmapPageAllocator::CMmapPageAllocator()
    {
    }
    
    CMmapPageAllocator::~CMmapPageAllocator()
    {
    }
    
    void * CMmapPageAllocator::SysAlloc(size_t size)
    {
        void * ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == NULL) {
            FT_LOG(FT_ERROR, "mmap failed, errno:%d, %s", errno, strerror(errno));
        }
        
        FT_LOG(FT_DEBUG, "mmap addr:%p, size:%zd", ptr, size);

        return ptr;
    }
    
    void CMmapPageAllocator::SysRelease(void * ptr, size_t size)
    {
        FT_LOG(FT_DEBUG, "munmap, addr:%p, size:%zd", ptr, size);
        if (!munmap(ptr, size)) {
            FT_LOG(FT_ERROR, "munmap failed, errno:%d, %s", errno, strerror(errno));
        }
    }
    
}