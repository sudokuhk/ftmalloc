/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_malloc_intf.h"
#include "ft_thread_cache.h"
#include "ft_mem_alloc_intf.h"

namespace ftmalloc
{
    void * __Malloc(size_t size)
    {
        IMemAlloc * allocator = CThreadCache::GetInstance().GetAllocator();
        return allocator->Malloc(size);
    }

    void * __ReAlloc(void * ptr, size_t size)
    {
        IMemAlloc * allocator = CThreadCache::GetInstance().GetAllocator();
        return allocator->ReAlloc(ptr, size);
    }

    void * __Calloc(size_t nmemb, size_t size)
    {
        IMemAlloc * allocator = CThreadCache::GetInstance().GetAllocator();
        return allocator->Calloc(nmemb, size);
    }

    void __Free(void * ptr)
    {
        IMemAlloc * allocator = CThreadCache::GetInstance().GetAllocator();
        return allocator->Free(ptr);
    }
}