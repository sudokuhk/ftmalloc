/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_MALLOC_ALIAS_H__
#define __FT_MALLOC_ALIAS_H__

#ifndef __THROW
#define __THROW
#endif

#define ALIAS(name)   __attribute__ ((alias (#name)))

/*
void* operator new(size_t size) throw (std::bad_alloc)
    ALIAS(ft_new);
void operator delete(void* p) __THROW
    ALIAS(ft_delete);
void* operator new[](size_t size) throw (std::bad_alloc)
    ALIAS(ft_newarray);
void operator delete[](void* p) __THROW
    ALIAS(ft_deletearray);
void* operator new(size_t size, const std::nothrow_t& nt) __THROW
    ALIAS(ft_new_nothrow);
void* operator new[](size_t size, const std::nothrow_t& nt) __THROW
    ALIAS(ft_newarray_nothrow);
void operator delete(void* p, const std::nothrow_t& nt) __THROW
    ALIAS(ft_delete_nothrow);
void operator delete[](void* p, const std::nothrow_t& nt) __THROW
    ALIAS(ft_deletearray_nothrow);
*/

extern "C" 
{
    void* malloc(size_t size) __THROW               ALIAS(ft_malloc);
    void free(void* ptr) __THROW                    ALIAS(ft_free);

/*
    void* realloc(void* ptr, size_t size) __THROW   ALIAS(ft_realloc);
    void* calloc(size_t n, size_t size) __THROW     ALIAS(ft_calloc);
    void cfree(void* ptr) __THROW                   ALIAS(ft_cfree);
    void* memalign(size_t align, size_t s) __THROW  ALIAS(ft_memalign);
    void* valloc(size_t size) __THROW               ALIAS(ft_valloc);
    void* pvalloc(size_t size) __THROW              ALIAS(ft_pvalloc);
    int posix_memalign(void** r, size_t a, size_t s) __THROW    ALIAS(ft_posix_memalign);
    int mallopt(int cmd, int value) __THROW         ALIAS(ft_mallopt);
    size_t malloc_size(void* p) __THROW             ALIAS(ft_malloc_size);
    size_t malloc_usable_size(void* p) __THROW      ALIAS(ft_malloc_size);
    */
}

#undef ALIAS

#endif
