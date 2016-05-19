/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_LOCK_H__
#define __FT_LOCK_H__

#include <pthread.h>

namespace ftmalloc
{    
    typedef pthread_mutex_t CMutexType;

    #define FT_MUTEX_INITIALIZER()      \
        PTHREAD_MUTEX_INITIALIZER

    #define FT_MUTEX_CREATE(mutex)      \
        pthread_mutex_init(&mutex, NULL);

    #define FT_MUTEX_DESTROY(mutex)      \
        pthread_mutex_destroy(&mutex);

    #define FT_MUTEX_LOCK(mutex)        \
        pthread_mutex_lock(&mutex)

    #define FT_MUTEX_UNLOCK(mutex)      \
        pthread_mutex_unlock(&mutex)

    class CAutoLock
    {
    public:
        CAutoLock(CMutexType & mutex)
            : mMutex(mutex)
        {
            FT_MUTEX_LOCK(mMutex);
        }

        ~CAutoLock()
        {
            FT_MUTEX_UNLOCK(mMutex);
        }
    private:
        CMutexType & mMutex;
    };
}

#endif