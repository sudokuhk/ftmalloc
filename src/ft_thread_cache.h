/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
**/

#ifndef __FT_THREAD_CACHE_H__
#define __FT_THREAD_CACHE_H__

#include <pthread.h>

namespace ftmalloc
{
    class IMemAlloc;
    
    class CThreadCache
    {
    public:
        ~CThreadCache();

        static CThreadCache & GetInstance();

        IMemAlloc * GetAllocator();
        
    private:
        CThreadCache();
        CThreadCache(const CThreadCache &);
        CThreadCache & operator=(const CThreadCache &);

        static CThreadCache sInstance;
        static pthread_mutex_t sMutex;

        pthread_key_t m_cKey;
    };
}

#endif
