/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_MALLOC_H__
#define __FT_MALLOC_H__

#ifndef __THROW
#define __THROW
#endif

#include <stddef.h>

#define HK_VERSION_MAJOR  1
#define HK_VERSION_MINOR  0
#define HK_VERSION_PATCH  ""
#define HK_VERSION_STRING "sudoku.huang's fast malloc 1.0"

#ifdef __cplusplus
namespace std {
struct nothrow_t;
}

extern "C" {
#endif

  const char * ft_version(int* major, int* minor, const char** patch) __THROW;

  void * ft_malloc(size_t size) __THROW;
  void * ft_malloc_skip_new_handler(size_t size) __THROW;
  void   ft_free(void* ptr) __THROW;
  void * ft_realloc(void* ptr, size_t size) __THROW;
  void * ft_calloc(size_t nmemb, size_t size) __THROW;
  void   ft_cfree(void* ptr) __THROW;

  void * ft_memalign(size_t __alignment, size_t __size) __THROW;
  int    ft_posix_memalign(void** ptr, size_t align, size_t size) __THROW;
  void * ft_valloc(size_t __size) __THROW;
  void * ft_pvalloc(size_t __size) __THROW;

  void   ft_malloc_stats(void) __THROW;
  int    ft_mallopt(int cmd, int value) __THROW;
#if 0
  struct mallinfo ft_mallinfo(void) __THROW;
#endif

  size_t ft_malloc_size(void* ptr) __THROW;

#ifdef __cplusplus
  int    ft_set_new_mode(int flag) __THROW;
  void * ft_new(size_t size);
  void * ft_new_nothrow(size_t size, const std::nothrow_t&) __THROW;
  void   ft_delete(void* p) __THROW;
  void   ft_delete_nothrow(void* p, const std::nothrow_t&) __THROW;
  void * ft_newarray(size_t size);
  void * ft_newarray_nothrow(size_t size, const std::nothrow_t&) __THROW;
  void   ft_deletearray(void* p) __THROW;
  void   ft_deletearray_nothrow(void* p, const std::nothrow_t&) __THROW;
}
#endif

#endif
