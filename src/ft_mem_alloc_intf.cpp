/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#include "ft_mem_alloc_intf.h"
#include "ft_malloc_slab.h"
#include "ft_cache_allocator.h"
#include "ft_mmap_page_allocator.h"
#include "ft_lock.h"
#include "ft_malloc_log.h"

namespace ftmalloc
{
    extern CMmapPageAllocator s_mmap_page_allocator;
    const size_t s_tc_page_bit = FT_PAGE_BIT;
    static CSlab<CCacheAllocator> s_mem_alloc_slab("cache_allocator", s_mmap_page_allocator, s_tc_page_bit);
    static CMutexType sCacheAllocateLock = FT_MUTEX_INITIALIZER();
    
    IMemAlloc * IMemAlloc::CreateMemAllocator()
    {
        FT_LOG(FT_INFO, "create memory allocator!");
        CAutoLock lock(sCacheAllocateLock);
        CCacheAllocator * allocator = s_mem_alloc_slab.AllocNode();
        return allocator;
    }

    void IMemAlloc::DestroyMemAllocator(IMemAlloc * &allocator)
    {
        FT_LOG(FT_INFO, "destroy memory allocator!");
        if (allocator != NULL) {
            CCacheAllocator * ptr = (CCacheAllocator *)allocator;
            s_mem_alloc_slab.ReleaseNode(ptr);
        }
    }
}
