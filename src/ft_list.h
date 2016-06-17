#ifndef __FT_LIST_H__
#define __FT_LIST_H__

#include <stdio.h>

namespace ftmalloc
{
#ifdef _MSC_VER
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))
#else
#define container_of(ptr, type, member) ({            \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#if defined(offsetof)
#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

    struct list_head {
        struct list_head * prev;
        struct list_head * next;
    };

    #define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_NODE(node)    \
    init_list_head(node)

#define INIT_LIST_HEAD(head)    \
    init_list_head(head)

    static inline void init_list_head(struct list_head *list)
    {
    	list->next = list;
    	list->prev = list;
    }

    static inline void __list_add(struct list_head *newnode,
			      struct list_head *prev,
			      struct list_head *next)
    {
    	next->prev      = newnode;
    	newnode->next   = next;
    	newnode->prev   = prev;
    	prev->next      = newnode;
    }

    static inline void list_add(struct list_head *newnode, struct list_head *head)
    {
    	__list_add(newnode, head, head->next);
    }

    static inline void list_add_tail(struct list_head *newnode, struct list_head *head)
    {
    	__list_add(newnode, head->prev, head);
    }

    static inline void __list_del(struct list_head * prev, struct list_head * next)
    {
    	next->prev = prev;
    	prev->next = next;
    }

    static inline void list_del(struct list_head *entry)
    {
    	__list_del(entry->prev, entry->next);
    	entry->next = NULL;
    	entry->prev = NULL;
    }

    static inline int list_is_last(const struct list_head *list,
				const struct list_head *head)
    {
    	return list->next == head;
    }

    static inline int list_empty(const struct list_head *head)
    {
    	return head->next == head;
    }

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)
	
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)
    
}
#endif //__RB_TREE_H__
