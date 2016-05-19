/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_cache_allocator.h"
#include "ft_malloc_log.h"
#include "ft_central_cache_mgr.h"

#include <string.h>
#include <errno.h>

namespace ftmalloc
{
#define MEM_INFO_TYPE                   int
#define MEM_INFO_LENGTH                 (sizeof(MEM_INFO_TYPE))//align.

#define ALLOC_BYTE_INFONUM              (1)
#define ALLOC_PAGE_INFONUM              (2)

#define GET_MEM_INFO(mem)               (*((MEM_INFO_TYPE *)(((size_t)(mem) - MEM_INFO_LENGTH))))
#define SET_MEM_INFO(mem, info)         do { (*((MEM_INFO_TYPE *)(mem))) = (info); } while (0)

#define RETURN_MEM_ADDR(mem)            ((void *)(((size_t)(mem)) + MEM_INFO_LENGTH))
#define GET_MEM_ADDR(mem)               ((void *)(((size_t)(mem)) - MEM_INFO_LENGTH))
#define GET_ALLOC_LENGTH(size, infonum) ((size) + infonum * (MEM_INFO_LENGTH))
#define IS_BIG_MEM_ALLOC(size)          (GET_ALLOC_LENGTH(size, ALLOC_BYTE_INFONUM))

#define GET_DATASIZE_BY_PAGE(pages)     (((pages) << FT_PAGE_BIT) - (ALLOC_PAGE_INFONUM * MEM_INFO_LENGTH))
#define GET_DATASIZE_BY_BYTE(bytes)     ((bytes) - (ALLOC_BYTE_INFONUM * MEM_INFO_LENGTH))

#define PAGE_CLAZZ                      (-1)

    CCacheAllocator::CCacheAllocator()
        : m_cFreeList()
        , m_llAllocSize(0)
        , m_llUsedSize(0)
        , m_llAllocPages(0)
    {
        for (size_t i = 0; i < kNumClasses; i++) {
            m_cFreeList[i].Init();
        }
    }
    
    CCacheAllocator::~CCacheAllocator()
    {
        for (size_t i = 0; i < kNumClasses; i++) {
            if (m_cFreeList[i].length() > 0) {
                FT_LOG(FT_DEBUG, "freelist[%zd].size = %zd", i, m_cFreeList[i].length());
                ReleaseToCentral(i, m_cFreeList[i].length());
            }
        }
    }

    void * CCacheAllocator::Malloc(size_t bytes)
    {
        FT_LOG(FT_DEBUG, "want size:%zd", bytes);
        void * addr = NULL;
        if (bytes == 0) {
            return addr;
        }
        
        if (IS_BIG_MEM_ALLOC(bytes) >= kMaxSize) {
            addr = PageAlloc(bytes);
        } else {
            addr = SmallAlloc(bytes);
        }
        FT_LOG(FT_DEBUG, "want size:%zd, addr:%p", bytes, addr);
        return addr;
    }

    void * CCacheAllocator::ReAlloc(void * oldptr, size_t bytes)
    {
        void * addr = NULL;
        
        if (oldptr == NULL) {
            addr = Malloc(bytes);
        } else if (bytes == 0) {
            addr = NULL;
        } else {
            size_t clazz    = GET_MEM_INFO(oldptr);
            void * realAddr = GET_MEM_ADDR(oldptr);
            size_t oldsize  = 0;

            if (clazz == PAGE_CLAZZ) {
                size_t pages    = GET_MEM_INFO(realAddr);
                realAddr        = GET_MEM_ADDR(realAddr);
                oldsize         = GET_DATASIZE_BY_PAGE(pages);
            } else {
                oldsize = GET_DATASIZE_BY_BYTE(CSizeMap::GetInstance().class_to_size(clazz));
            }

            if (bytes > oldsize) {
                void * addr = Malloc(bytes);
                memcpy(addr, oldptr, oldsize);
                Free(oldptr);
            } else {
                addr = oldptr;
            }
        }

        return addr;
    }

    void * CCacheAllocator::Calloc(size_t nmemb, size_t size)
    {
        size_t needsize = nmemb * size;
        if (needsize != 0 && needsize / size != nmemb) {
            return NULL;
        }

        void * addr = Malloc(needsize);
        if (addr == NULL) {
            errno = ENOMEM;
        } else {
            memset(addr, 0, needsize);
        }

        return addr;
    }

    void CCacheAllocator::Free(void * ptr)
    {
        if (ptr == NULL) {
            FT_LOG(FT_INFO, "Error, _Free invalid ptr:%p\n", ptr);
            return;
        }

        size_t clazz = GET_MEM_INFO(ptr);
        size_t freeSize = 0;
        
        void * realAddr = GET_MEM_ADDR(ptr);
        FT_LOG(FT_DEBUG, "Get clazz info from memory:%zd, addr:%p, real:%p", clazz, ptr, realAddr);

        if (clazz != PAGE_CLAZZ) {
            freeSize = CSizeMap::GetInstance().class_to_size(clazz);
            CFreeList & list = m_cFreeList[clazz];
            list.Push(realAddr);

            FT_LOG(FT_DEBUG, "clazz:%zd, length:%zd, max_length:%zd!", clazz, list.length(), list.max_length());
            
            if (list.length() >= list.max_length()) {
                list.set_max_length(list.max_length() >> 1);
                ReleaseToCentral(clazz, list.length() - list.max_length());
            }
        } else {
            size_t pages = GET_MEM_INFO(realAddr);
            realAddr = GET_MEM_ADDR(realAddr);
            freeSize = pages << FT_PAGE_BIT;

            CCentralCacheMgr::GetInstance().FreePages(realAddr, pages);
        }
    }

    void * CCacheAllocator::SmallAlloc(size_t bytes)
    {
        CSizeMap sizemap    = CSizeMap::GetInstance();

        size_t allocSize    = GET_ALLOC_LENGTH(bytes, ALLOC_BYTE_INFONUM);
        size_t cl           = sizemap.SizeClass(allocSize);
        size_t size         = sizemap.class_to_size(cl);

        FT_LOG(FT_DEBUG, "SmallAlloc, want:%zd, c1:%zd, size:%zd", bytes, cl, size);

        void * allocAddr = NULL;

        CFreeList & list = m_cFreeList[cl];
        FT_LOG(FT_DEBUG, "clazz:%zd, freeobj:%zd", cl, list.length());
        if (list.empty()) {
            allocAddr = FetchMemFromCentral(cl, size);
        } else {
            allocAddr = list.Pop();
        }
        FT_LOG(FT_DEBUG, "object addr:%p", allocAddr);
        
        m_llUsedSize += size;
        SET_MEM_INFO(allocAddr, cl);

        void * retAddr = RETURN_MEM_ADDR(allocAddr);
        FT_LOG(FT_DEBUG, "return addr:%p", retAddr);

        return retAddr;
    }
    
    void * CCacheAllocator::PageAlloc(size_t bytes)
    {
        size_t allocSize = GET_ALLOC_LENGTH(bytes, ALLOC_PAGE_INFONUM);
        size_t needPages = (allocSize >> FT_PAGE_BIT) + ((allocSize & (FT_PAGE_BIT - 1)) > 0 ? 1 : 0);

        FT_LOG(FT_DEBUG, "want size:%zd, realsize:%zd, pages:%zd", bytes, allocSize, needPages);

        void * allocAddr = CCentralCacheMgr::GetInstance().AllocPages(needPages);
        FT_LOG(FT_DEBUG, "want size:%zd, realsize:%zd, pages:%zd, addr:%p", bytes, allocSize, needPages, allocAddr);
        
        m_llAllocPages += needPages;

        void * retAddr = allocAddr;

        SET_MEM_INFO(retAddr, needPages);
        retAddr = RETURN_MEM_ADDR(retAddr);

        SET_MEM_INFO(retAddr, PAGE_CLAZZ);
        retAddr = RETURN_MEM_ADDR(retAddr);

        FT_LOG(FT_DEBUG, "return addr:%p", retAddr);

        return retAddr;
    }
    
    void * CCacheAllocator::FetchMemFromCentral(size_t clazz, size_t size)
    {
        CFreeList & list = m_cFreeList[clazz];
        //ASSERT(list.empty());
        FT_LOG(FT_DEBUG, "FetchMemFromCentral, clz:%zd, size:%zd", clazz, size);

        CSizeMap sizemap = CSizeMap::GetInstance();

        const size_t batch_size = sizemap.num_objects_to_move(clazz);
        const size_t num_to_move = FT_MIN(list.max_length(), batch_size);
        FT_LOG(FT_DEBUG, "FetchMemFromCentral, batchSize:%zd, list.length:%zd, num_to_move:%zd", 
            batch_size, list.max_length(), num_to_move);
        
        void *start, *end;
        size_t fetch_count = CCentralCacheMgr::GetInstance().RemoveRange(clazz, &start, &end, num_to_move);
        FT_LOG(FT_DEBUG, "FetchMemFromCentral, alloc nodes from central:%zd, start:%p, end:%p", fetch_count, start, end);

        m_llAllocSize += fetch_count * size;
        if (--fetch_count >= 0) {
            list.PushRange(fetch_count, SLL_Next(start), end);
        }

        if (list.max_length() < batch_size) {
            FT_LOG(FT_DEBUG, "FetchMemFromCentral, set list max size[%zd] = %zd", clazz, list.max_length() << 1);
            list.set_max_length(list.max_length() << 1);
        }

        return start;
    }
    
    void   CCacheAllocator::ReleaseToCentral(size_t clazz, size_t N)
    {
        FT_LOG(FT_DEBUG, "clz:%zd, returnsize:%zd", clazz, N);
        CFreeList & list = m_cFreeList[clazz];
        
        if (list.length() < N) {
            N = list.length();
        }

        CSizeMap & sizemap = CSizeMap::GetInstance();

        size_t node_size = sizemap.class_to_size(clazz);
        size_t batch_size = CSizeMap::GetInstance().num_objects_to_move(clazz);

        m_llAllocSize -= N * node_size;

        while (N > batch_size) {
            void *tail, *head;
            list.PopRange(batch_size, &head, &tail);
            CCentralCacheMgr::GetInstance().InsertRange(clazz, head, tail, batch_size);
            N -= batch_size;
        }

        void * start, *end;
        list.PopRange(N, &start, &end);
        CCentralCacheMgr::GetInstance().InsertRange(clazz, start, end, N);
    }
    
}
