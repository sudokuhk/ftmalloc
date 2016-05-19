/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_PAGE_MGR_H__
#define __FT_PAGE_MGR_H__

#include "ft_malloc_util.h"
#include "ft_rb_tree.h"

#include <stddef.h>

namespace ftmalloc
{   
    class CPageMgr
    {
    public:
        struct SPageInfo
        {
            enum {
                E_MEM_SOURCE_OFF = 0,
            };

            enum {
                E_SRC_MMAP  = 0,
                E_SRC_SBRK  = 1,
            };

            void SetFlag(size_t offset)
            {
                flag |= (1 << offset);
            }

            void UnSetFlag(size_t offset)
            {
                flag &= ~(1 << offset);
            }

            bool FromBrk()
            {
                return (flag & (1 << E_MEM_SOURCE_OFF)) == E_SRC_SBRK;
            }

            size_t BeginAddress()
            {
                return base_address;
            }

            size_t EndAddress() 
            {
                return base_address + (page_count << FT_PAGE_BIT);
            }
            
            size_t base_address;
            size_t page_count;
            size_t flag;

            struct rb_node address_node;
            struct rb_node free_node;
        };

        struct SIndexInfo
        {
            size_t page_count;
            struct rb_node hash_node;
            struct rb_root free_tree;
        };

        struct SCountInfo
        {
            size_t page_count;
            struct rb_node count_node;
        };

        struct SHashNode
        {
            struct rb_root hash_tree;
        };
        
    public:
        static CPageMgr & GetInstance();
        
        ~CPageMgr();
        void * AllocPages(size_t wantpages);
        void ReleasePages(void * ptr, size_t pages);

    private:
        int AllocPagesFromSys(size_t pages);
        int ReleasePagesToSys(void * addr, size_t pages, bool frommmap);

        void InsertPageInfo(void * addr, size_t pagecount, bool frommmap = false);
        void InsertPageInfo(struct SPageInfo * pageInfo);
        void InsertIndexTreeIfNeed(size_t pagecount);
        void InsertCountTreeIfNeed(size_t pagecount);
        void InsertAddressTree(struct SPageInfo * pageInfo);
        void InsertFreeTree(struct SPageInfo * pageInfo);

        void RemovePageInfo(struct SPageInfo * pageInfo);
        void RemoveIndexTreeIfNeed(size_t pagecount);
        void RemoveCountTreeIfNeed(size_t pagecount);
        void RemoveAddressTree(struct SPageInfo * pageInfo);
        void RemoveFreeTree(struct SPageInfo * pageInfo);

        struct SPageInfo * GetPageInfoByAddress(size_t address);
        struct SPageInfo * GetPageInfo(struct SIndexInfo * indexInfo);
        struct SIndexInfo * GetIndexInfo(size_t pagecount);
        size_t GetBestFitIndex(size_t wantpages);
        size_t GetMaxContinuePages();

        void ReleasePageIfNeed();
        void * DecPageCount(struct SPageInfo * pageInfo, size_t freesize);
    private:
        struct SPageInfo * AllocPageInfo();
        struct SIndexInfo * AllocIndexInfo();
        struct SCountInfo * AllocCountInfo();
        
        void ReleasePageInfo(struct SPageInfo * &pageInfo);
        void ReleaseIndexInfo(struct SIndexInfo * &indexInfo);
        void ReleaseCountInfo(struct SCountInfo * &countInfo);

        bool TopDownMMap();
        size_t Hash(size_t pagecount);
        struct SHashNode * GetHashNode(size_t pagecount);

    private:
        typedef void * (CPageMgr::*RbGetObjectFunc)(void * node);
        typedef rb_node * (CPageMgr::*RbGetNodeFunc)(void * object);
        typedef size_t (CPageMgr::*RbSearchFunc)(const void * lhs, const void * rhs);
        typedef size_t (CPageMgr::*RbInsertFunc)(const void * lhs, const void * rhs);
        
    private:
        void * RbSearch(struct rb_root *root, void * object, 
            RbGetObjectFunc getObject, RbSearchFunc search);
        int RbInsert(struct rb_root *root, void *data, 
            RbGetObjectFunc getObject, RbGetNodeFunc getNode, RbInsertFunc compare);
        void RbRemove(rb_root * root, void * object, RbGetNodeFunc getNode);

    private:
        void * HashTreeGetObject(void * node);
        rb_node * HashTreeGetRbNode(void * object);
        size_t HashTreeSearch(const void * lhs, const void * rhs);
        size_t HashTreeInsert(const void * lhs, const void * rhs);

    private:
        void * AddressTreeGetObject(void * node);
        rb_node * AddressTreeGetRbNode(void * object);
        size_t AddressTreeSearch(const void * lhs, const void * rhs);
        size_t AddressTreeInsert(const void * lhs, const void * rhs);

    private:
        void * CountTreeGetObject(void * node);
        rb_node * CountTreeGetRbNode(void * object);
        size_t CountTreeSearch(const void * lhs, const void * rhs);
        size_t CountTreeInsert(const void * lhs, const void * rhs);

    private:
        void * FreeTreeGetObject(void * node);
        rb_node * FreeTreeGetRbNode(void * object);
        size_t FreeTreeSearch(const void * lhs, const void * rhs);
        size_t FreeTreeInsert(const void * lhs, const void * rhs);
    private:
        CPageMgr();
        CPageMgr(const CPageMgr &);
        CPageMgr & operator=(const CPageMgr &);

        static CPageMgr sInstance;

    private:
        enum {
            E_HASH_SIZE = 4096,

            E_MMAP_DIRECTION_BIT = 0,
        };
        struct SHashNode m_cHash[E_HASH_SIZE];
        struct rb_root   m_cAddressTree;
        struct rb_root   m_cCountTree;

        size_t m_llTopBrkAddress;

        size_t m_iAddressTreeSize;
        size_t m_iMaxContinuePages;
        size_t m_iFreePages;
        size_t m_iSbrkPages;
        size_t m_iMmapPages;
        size_t m_iFlags;
    };
}

#endif
