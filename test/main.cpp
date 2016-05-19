/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#if 0
#include "ftmalloc.h"

void * thread_func(void * args)
{
    printf(" T1 begin!, test ft_malloc!!\n");
    getchar();

    printf("T1 alloc int!\n");
    int * p = (int *)ft_malloc(sizeof(int));
    getchar();

    printf("T1 assign value to int!\n");
    *p = 10;
    getchar();

    printf("T1 free int!\n");
    ft_free((void *)p);
    getchar();
   
}
#else
void * thread_func(void * args)
{
    printf(" T1 begin!, test malloc!!\n");
    getchar();

    printf("T1 alloc int!\n");
    int * p = (int *)malloc(sizeof(int));
    getchar();

    printf("T1 assign value to int!\n");
    *p = 10;
    getchar();

    printf("T1 free int!\n");
    free((void *)p);
    getchar();  
}
#endif

int main()
{
    pthread_t pt;
    pthread_create(&pt, NULL, thread_func, NULL);
    pthread_join(pt, NULL);
    return 0;
}
