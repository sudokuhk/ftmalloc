/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_CACHE_ALLOCATOR_H__
#define __FT_CACHE_ALLOCATOR_H__

#include "ft_mem_alloc_intf.h"
#include "ft_sizemap.h"
#include "ft_free_list.h"

namespace ftmalloc
{
    class CCacheAllocator : public IMemAlloc
    {
    public:
        CCacheAllocator();
        virtual ~CCacheAllocator();

        virtual void * Malloc(size_t);
        virtual void * ReAlloc(void *, size_t);
        virtual void * Calloc(size_t, size_t);
        virtual void Free(void *);

    private:
        void * SmallAlloc(size_t bytes);
        void * PageAlloc(size_t bytes);
        void * FetchMemFromCentral(size_t clazz, size_t size);
        void   ReleaseToCentral(size_t clazz, size_t N);
        void ShowCacheInfo();
        
    private:
        CCacheAllocator(const CCacheAllocator &);
        CCacheAllocator & operator=(const CCacheAllocator &);

    private:
        CFreeList m_cFreeList[kNumClasses];
        size_t m_llAllocSize;
        size_t m_llUsedSize;
        size_t m_llAllocPages;
    };
}

#endif