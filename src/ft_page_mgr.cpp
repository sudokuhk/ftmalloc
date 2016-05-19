/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_page_mgr.h"
#include "ft_sbrk_page_allocator.h"
#include "ft_mmap_page_allocator.h"
#include "ft_malloc_slab.h"
#include "ft_free_list.h"
#include "ft_malloc_log.h"

#include <sys/resource.h>

namespace ftmalloc
{
#ifndef FT_PAGE_IDLE_RATE
#define FT_PAGE_IDLE_RATE   30
#endif

#ifndef FT_PAGE_MIN     
#define FT_PAGE_MIN         20
#endif

    extern CMmapPageAllocator s_mmap_page_allocator;
    extern CSbrkPageAllocator s_sbrk_page_allocator;
    
    static CSlab<CPageMgr::SPageInfo>  s_pageinfo_slab("page_info", s_mmap_page_allocator);
    static CSlab<CPageMgr::SIndexInfo> s_indexinfo_slab("index_info", s_mmap_page_allocator);
    static CSlab<CPageMgr::SCountInfo> s_countinfo_slab("count_info", s_mmap_page_allocator);
    
    CPageMgr CPageMgr::sInstance;

    CPageMgr & CPageMgr::GetInstance()
    {
        return sInstance;
    }
        
    CPageMgr::CPageMgr()
        : m_llTopBrkAddress(0)
        , m_iAddressTreeSize(0)
        , m_iMaxContinuePages(0)
        , m_iFreePages(0)
        , m_iSbrkPages(0)
        , m_iMmapPages(0)
        , m_iFlags(0)
    {
        RB_ROOT_INIT(m_cAddressTree);
        RB_ROOT_INIT(m_cCountTree);

        for (int i = 0; i < E_HASH_SIZE; i++) {
            RB_ROOT_INIT(m_cHash[i].hash_tree);
        }

        //mmap direction. top-down OR down-top.
        struct rlimit rlim;
        if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
            FT_LOG(FT_INFO, "process mmap direction:%s!", (rlim.rlim_cur == RLIM_INFINITY ? "down-top" : "top-down"));
            
            if (rlim.rlim_cur == RLIM_INFINITY) {
                m_iFlags &= ~(1 << E_MMAP_DIRECTION_BIT);
            } else {
                m_iFlags |= (1 << E_MMAP_DIRECTION_BIT);
            }
        }
    }

    CPageMgr::~CPageMgr()
    {
        FT_LOG(FT_DEBUG, "page mgr deconstructor!");
    }

    void * CPageMgr::AllocPages(size_t wantpages)
    {   
        void * addr = NULL;

        FT_LOG(FT_DEBUG, "want page:%zd, max continue:%zd", wantpages, m_iMaxContinuePages);
        
        if (m_iMaxContinuePages < wantpages) {
            size_t allocpages = FT_MAX(FT_PAGE_MIN, wantpages);
            AllocPagesFromSys(allocpages);
        }

        FT_LOG(FT_DEBUG, "want page:%zd, max continue:%zd", wantpages, m_iMaxContinuePages);
        
        size_t bestIndex = GetBestFitIndex(wantpages);
        struct SIndexInfo * indexInfo = GetIndexInfo(bestIndex);
        struct SPageInfo * pageInfo   = GetPageInfo(indexInfo);

        FT_LOG(FT_DEBUG, "want page:%zd, bestIndex:%zd", wantpages, bestIndex);
        if (pageInfo->page_count == wantpages) {
            addr = (void *)pageInfo->base_address;

            RemovePageInfo(pageInfo);
            m_iAddressTreeSize --;
            
            FT_LOG(FT_DEBUG, "after alloc, address tree size:%zd", m_iAddressTreeSize);
        } else {
            size_t offset = (pageInfo->page_count - wantpages) << FT_PAGE_BIT;
            
            addr = (void *)(pageInfo->base_address + offset);
            RemoveFreeTree(pageInfo);
            RemoveCountTreeIfNeed(pageInfo->page_count);
            RemoveIndexTreeIfNeed(pageInfo->page_count);

            pageInfo->page_count  -= wantpages;
            InsertIndexTreeIfNeed(pageInfo->page_count);
            InsertCountTreeIfNeed(pageInfo->page_count);
            InsertFreeTree(pageInfo);
        }
        
        m_iFreePages -= wantpages;
        m_iMaxContinuePages = GetMaxContinuePages();

        FT_LOG(FT_DEBUG, "want page:%zd, max continue:%zd, ret:%p", wantpages, m_iMaxContinuePages, addr);
        return addr;
    }
    
    void CPageMgr::ReleasePages(void * ptr, size_t pages)
    {
        FT_LOG(FT_DEBUG, "release, addr:%p, pages:%zd", ptr, pages);
        
        size_t addr = (size_t)ptr;
        bool frommmap = (m_llTopBrkAddress < addr) ? true : false;
        InsertPageInfo(ptr, pages, frommmap);

        ReleasePageIfNeed();
    }

    void CPageMgr::ReleasePageIfNeed()
    {
        size_t idleRate = m_iFreePages * 100 / (m_iMmapPages + m_iSbrkPages);
        FT_LOG(FT_DEBUG, "freepages:%zd, mmap:%zd, sbrk:%zd, idle rate:%zd",
            m_iFreePages, m_iMmapPages, m_iSbrkPages, idleRate);
            
        size_t freesize = m_iFreePages * FT_PAGE_IDLE_RATE / 100;
        FT_LOG(FT_DEBUG, "need release %zd pages!", freesize);

        if ((m_iFreePages - freesize) < FT_PAGE_MIN || idleRate < FT_PAGE_IDLE_RATE) {
            FT_LOG(FT_INFO, "no need release pages!, idlerate:%d, min_pages:%d",
                FT_PAGE_IDLE_RATE, FT_PAGE_MIN);
            return;
        }
        
        if (freesize <= 0) {
            return;
        }

        struct SPageInfo * pageInfo = (struct SPageInfo *)GetPageInfoByAddress(m_llTopBrkAddress - 1);
        if (pageInfo == NULL) {
            FT_LOG(FT_INFO, "sbrk top address is inuse,  can't brk down!");
        } else {
            FT_LOG(FT_DEBUG, "sbrk top info, addr:%p, pagecount:%zd", 
                (void*)pageInfo->base_address, pageInfo->page_count);
            size_t sbrk_freesize = FT_MIN(pageInfo->page_count, freesize);
            if (sbrk_freesize > 0) {
                void * brk_addr = DecPageCount(pageInfo, sbrk_freesize);
                ReleasePagesToSys(brk_addr, sbrk_freesize, false);
            }
            freesize -= sbrk_freesize;
        }

        if (freesize > 0) {
            rb_node * right = rb_last(&m_cAddressTree);
            while (right) {
                struct SPageInfo * pInfo = (struct SPageInfo *)AddressTreeGetObject(right);
                if (pInfo->FromBrk()) {
                    break;
                }

                size_t mmap_freesize = FT_MIN(freesize, pInfo->page_count);
                void * mmap_addr = DecPageCount(pInfo, mmap_freesize);
                ReleasePagesToSys(mmap_addr, mmap_freesize, true);

                freesize -= mmap_freesize;

                if (freesize == 0) {
                    break;
                }

                right = rb_last(&m_cAddressTree);
            }
        }

        FT_LOG(FT_DEBUG, "freepages:%zd, mmap:%zd, sbrk:%zd", m_iFreePages, m_iMmapPages, m_iSbrkPages);
        FT_LOG(FT_DEBUG, "sbrk top:%p", (void *)m_llTopBrkAddress);
        
    }

    void * CPageMgr::DecPageCount(struct SPageInfo * pageInfo, size_t freesize)
    {
        FT_LOG(FT_DEBUG, "dec pagecount, remove curr page info!");
        RemoveFreeTree(pageInfo);
        RemoveAddressTree(pageInfo);
        RemoveCountTreeIfNeed(pageInfo->page_count);
        RemoveIndexTreeIfNeed(pageInfo->page_count);
        m_iFreePages -= pageInfo->page_count;

        void * addr = NULL;
        bool fromBrk = pageInfo->FromBrk();

        FT_LOG(FT_DEBUG, "dec pagecount, pagesize:%zd, sub freesize:%zd!", pageInfo->page_count, freesize);
        if (freesize == pageInfo->page_count) {
            addr = (void *)pageInfo->base_address;
            ReleasePageInfo(pageInfo);
        } else {
            size_t off = pageInfo->page_count - freesize;
            addr = (void *)(pageInfo->base_address + (off << FT_PAGE_BIT));
            pageInfo->page_count -= freesize;
            InsertPageInfo(pageInfo);
        }

        if (fromBrk) {
            m_llTopBrkAddress = (size_t)addr;
        }

        return addr;
    }

    int CPageMgr::AllocPagesFromSys(size_t pages)
    {
        void * ptr  = NULL;
        size_t size = pages << FT_PAGE_BIT;
        bool from_mmap = false;
        
        ptr = s_sbrk_page_allocator.SysAlloc(size);
        if (ptr == NULL) {
            ptr = s_mmap_page_allocator.SysAlloc(size);
            if (ptr != NULL) {
                from_mmap = true;
            } else {
                return -1;
            }
        } 
        
        if (ptr != NULL) {
            if (!from_mmap) {
                if ((size_t)ptr + size > m_llTopBrkAddress) {
                    m_llTopBrkAddress = (size_t)ptr + size;
                }
                m_iSbrkPages += pages;
            } else {
                m_iMmapPages += pages;
            }
            InsertPageInfo(ptr, pages, from_mmap);
        }
        FT_LOG(FT_DEBUG, "sbrk top:%p", (void *)m_llTopBrkAddress);
        
        return 0;
    }
    
    int CPageMgr::ReleasePagesToSys(void * addr, size_t pages, bool frommmap)
    {
        size_t size = pages << FT_PAGE_BIT;
        
        if (frommmap) {
            s_mmap_page_allocator.SysRelease(addr, size);
            m_iMmapPages -= pages;
        } else {
            s_sbrk_page_allocator.SysRelease(addr, size);
            m_iSbrkPages -= pages;
        }
    }

    void CPageMgr::InsertPageInfo(void * addr, size_t pagecount, bool frommmap)
    {
        struct SPageInfo * pageInfo = AllocPageInfo();
        RB_NODE_INIT(pageInfo->address_node);
        RB_NODE_INIT(pageInfo->free_node);
        pageInfo->base_address = (size_t)addr;
        pageInfo->page_count   = pagecount;
        if (frommmap) {
            pageInfo->UnSetFlag(SPageInfo::E_MEM_SOURCE_OFF);
        } else {
            pageInfo->SetFlag(SPageInfo::E_MEM_SOURCE_OFF);
        }

        FT_LOG(FT_DEBUG, "addr:%p, pages:%zd, mmap:%d", addr, pagecount, frommmap);
        FT_LOG(FT_DEBUG, "mgr info, address tree size:%zd", m_iAddressTreeSize);
        
        if (m_iAddressTreeSize == 0) {
            InsertPageInfo(pageInfo);
            m_iMaxContinuePages = pageInfo->page_count;
            m_iAddressTreeSize ++;
        } else if (m_iAddressTreeSize == 1) {
            rb_node * node = rb_first(&m_cAddressTree);
            struct SPageInfo * pInfo = (struct SPageInfo *)AddressTreeGetObject(node);

            if (pInfo->flag == pageInfo->flag && 
                (pInfo->BeginAddress() == pageInfo->EndAddress() ||
                pageInfo->BeginAddress() == pInfo->EndAddress())) {
                RemoveFreeTree(pInfo);
                RemoveCountTreeIfNeed(pInfo->page_count);
                RemoveIndexTreeIfNeed(pInfo->page_count);

                pInfo->base_address = FT_MIN(pInfo->base_address, pInfo->base_address);
                pInfo->page_count  += pageInfo->page_count;
                InsertIndexTreeIfNeed(pInfo->page_count);
                InsertCountTreeIfNeed(pInfo->page_count);
                InsertFreeTree(pInfo);

                ReleasePageInfo(pageInfo);
                
                m_iMaxContinuePages = pInfo->page_count;
            } else {
                InsertPageInfo(pageInfo);
                m_iMaxContinuePages = FT_MAX(pageInfo->page_count, pInfo->page_count);
                m_iAddressTreeSize ++;
            }
        } else {
            struct SPageInfo * prevInfo = GetPageInfoByAddress(pageInfo->BeginAddress() - 1);
            struct SPageInfo * nextInfo = GetPageInfoByAddress(pageInfo->EndAddress());

            if (prevInfo != NULL && prevInfo->flag == pageInfo->flag) {
                pageInfo->base_address  = prevInfo->base_address;
                pageInfo->page_count   += prevInfo->page_count;
                
                RemovePageInfo(prevInfo);
                m_iAddressTreeSize --;
                prevInfo = NULL;
            }

            if (nextInfo != NULL && nextInfo->flag == pageInfo->flag) {
                pageInfo->page_count   += nextInfo->page_count;
                
                RemovePageInfo(nextInfo);
                m_iAddressTreeSize --;
                nextInfo = NULL;
            }

            InsertPageInfo(pageInfo);
            m_iMaxContinuePages = FT_MAX(pageInfo->page_count, m_iMaxContinuePages);
            m_iAddressTreeSize ++;
        }

        m_iFreePages += pagecount;
        FT_LOG(FT_INFO, "max continut:%zd, address tree node:%zd, free:%zd", 
            m_iMaxContinuePages, m_iAddressTreeSize, m_iFreePages);
    }

    void CPageMgr::InsertPageInfo(struct SPageInfo * pageInfo)
    {
        InsertIndexTreeIfNeed(pageInfo->page_count);
        InsertCountTreeIfNeed(pageInfo->page_count);
        InsertAddressTree(pageInfo);
        InsertFreeTree(pageInfo);
    }

    void CPageMgr::InsertIndexTreeIfNeed(size_t pagecount)
    {
        struct SHashNode * hashNode     = GetHashNode(pagecount);
        struct SIndexInfo * indexInfo   = (struct SIndexInfo *)RbSearch(&hashNode->hash_tree, 
            &pagecount, &CPageMgr::HashTreeGetObject, &CPageMgr::HashTreeSearch);
        
        if (indexInfo == NULL) {
            indexInfo = AllocIndexInfo();

            RB_ROOT_INIT(indexInfo->free_tree);
            RB_NODE_INIT(indexInfo->hash_node);
            indexInfo->page_count = pagecount;

            RbInsert(&hashNode->hash_tree, indexInfo, &CPageMgr::HashTreeGetObject, 
                &CPageMgr::HashTreeGetRbNode, &CPageMgr::HashTreeInsert);
        }
    }

    void CPageMgr::InsertCountTreeIfNeed(size_t pagecount)
    {
        struct SCountInfo * countInfo = (struct SCountInfo *)RbSearch(&m_cCountTree, 
                &pagecount, &CPageMgr::CountTreeGetObject, &CPageMgr::CountTreeSearch);
        
        if (countInfo == NULL) {
            countInfo = AllocCountInfo();

            countInfo->page_count = pagecount;
            RB_NODE_INIT(countInfo->count_node);

            RbInsert(&m_cCountTree, countInfo, &CPageMgr::CountTreeGetObject, 
                &CPageMgr::CountTreeGetRbNode, &CPageMgr::CountTreeInsert);
        }
    }
    
    void CPageMgr::InsertAddressTree(struct SPageInfo * pageInfo)
    {
        RbInsert(&m_cAddressTree, pageInfo, &CPageMgr::AddressTreeGetObject, 
            &CPageMgr::AddressTreeGetRbNode, &CPageMgr::AddressTreeInsert);
    }
    
    void CPageMgr::InsertFreeTree(struct SPageInfo * pageInfo)
    {
        struct SHashNode * hashNode     = GetHashNode(pageInfo->page_count);
        struct SIndexInfo * indexInfo   = (struct SIndexInfo *)RbSearch(&hashNode->hash_tree, 
            &pageInfo->page_count, &CPageMgr::HashTreeGetObject, &CPageMgr::HashTreeSearch);

        if (indexInfo == NULL) {
            //ERROR!
        }

        RbInsert(&indexInfo->free_tree, pageInfo, &CPageMgr::FreeTreeGetObject, 
            &CPageMgr::FreeTreeGetRbNode, &CPageMgr::FreeTreeInsert);
    }

    void CPageMgr::RemovePageInfo(struct SPageInfo * pageInfo)
    {
        RemoveFreeTree(pageInfo);
        RemoveAddressTree(pageInfo);
        RemoveCountTreeIfNeed(pageInfo->page_count);
        RemoveIndexTreeIfNeed(pageInfo->page_count);

        ReleasePageInfo(pageInfo);
    }
    
    void CPageMgr::RemoveIndexTreeIfNeed(size_t pagecount)
    {
        struct SHashNode * hashNode     = GetHashNode(pagecount);
        struct SIndexInfo * indexInfo   = (struct SIndexInfo *)RbSearch(&hashNode->hash_tree, 
                &pagecount, &CPageMgr::HashTreeGetObject, &CPageMgr::HashTreeSearch);

        if (RB_EMPTY_ROOT(&indexInfo->free_tree)) {
            RbRemove(&hashNode->hash_tree, indexInfo, &CPageMgr::HashTreeGetRbNode);
            ReleaseIndexInfo(indexInfo);
        }
    }
    
    void CPageMgr::RemoveCountTreeIfNeed(size_t pagecount)
    {
        struct SHashNode * hashNode     = GetHashNode(pagecount);
        struct SIndexInfo * indexInfo   = (struct SIndexInfo *)RbSearch(&hashNode->hash_tree, 
            &pagecount, &CPageMgr::HashTreeGetObject, &CPageMgr::HashTreeSearch);

        if (RB_EMPTY_ROOT(&indexInfo->free_tree)) {
            struct SCountInfo * countInfo = (struct SCountInfo *)RbSearch(&m_cCountTree, 
                &pagecount, &CPageMgr::CountTreeGetObject, &CPageMgr::CountTreeSearch);
            
            RbRemove(&m_cCountTree, countInfo, &CPageMgr::CountTreeGetRbNode);
            ReleaseCountInfo(countInfo);
        }
    }
    
    void CPageMgr::RemoveAddressTree(struct SPageInfo * pageInfo)
    {
        RbRemove(&m_cAddressTree, pageInfo, &CPageMgr::AddressTreeGetRbNode);
    }
    
    void CPageMgr::RemoveFreeTree(struct SPageInfo * pageInfo)
    {
        struct SHashNode * hashNode     = GetHashNode(pageInfo->page_count);
        struct SIndexInfo * indexInfo   = (struct SIndexInfo *)RbSearch(&hashNode->hash_tree, 
            &pageInfo->page_count, &CPageMgr::HashTreeGetObject, &CPageMgr::HashTreeSearch);

        RbRemove(&indexInfo->free_tree, pageInfo, &CPageMgr::FreeTreeGetRbNode);
    }

    struct CPageMgr::SPageInfo * CPageMgr::GetPageInfoByAddress(size_t address)
    {   
        return (struct SPageInfo *)RbSearch(&m_cAddressTree, &address, 
            &CPageMgr::AddressTreeGetObject, &CPageMgr::AddressTreeSearch);
    }

    struct CPageMgr::SIndexInfo * CPageMgr::GetIndexInfo(size_t pagecount)
    {
        struct SHashNode * hashNode     = GetHashNode(pagecount);
        return (struct SIndexInfo *)RbSearch(&hashNode->hash_tree, &pagecount, 
            &CPageMgr::HashTreeGetObject, &CPageMgr::HashTreeSearch);
    }

    struct CPageMgr::SPageInfo * CPageMgr::GetPageInfo(struct SIndexInfo * indexInfo)
    {
        rb_node * first = rb_first(&indexInfo->free_tree);
        return (struct SPageInfo *)FreeTreeGetObject(first);
    }

    size_t CPageMgr::GetBestFitIndex(size_t wantpages)
    {
        struct rb_node * node = m_cCountTree.rb_node;
        struct SCountInfo * bestCountInfo = NULL;
            
        while (node) {
            struct SCountInfo * countInfo = (struct SCountInfo *)CountTreeGetObject(node);
            
            if (countInfo->page_count >= wantpages) {
                bestCountInfo = countInfo;
                node = node->rb_left;
            } else {
                if (bestCountInfo != NULL) {
                    break;
                }
                node = node->rb_right;
            }
        }

        return bestCountInfo->page_count;
    }

    size_t CPageMgr::GetMaxContinuePages()
    {
        struct rb_node * node = m_cCountTree.rb_node;
        struct rb_node * find = node;
            
        while (node) {
            find = node;
            node = node->rb_right;
        }

        if (find == NULL) {
            return 0;
        }
        
        struct SCountInfo * countInfo = (struct SCountInfo *)CountTreeGetObject(find);
        return countInfo->page_count;
    }
    
    CPageMgr::SPageInfo * CPageMgr::AllocPageInfo()
    {
        return s_pageinfo_slab.AllocNode();
    }
    
    CPageMgr::SIndexInfo * CPageMgr::AllocIndexInfo()
    {
        return s_indexinfo_slab.AllocNode();
    }

    CPageMgr::SCountInfo * CPageMgr::AllocCountInfo()
    {
        return s_countinfo_slab.AllocNode();
    }
    
    void CPageMgr::ReleasePageInfo(struct SPageInfo * &pageInfo)
    {
        s_pageinfo_slab.ReleaseNode(pageInfo);
    }
    
    void CPageMgr::ReleaseIndexInfo(struct SIndexInfo * &indexInfo)
    {
         s_indexinfo_slab.ReleaseNode(indexInfo);
    }

    void CPageMgr::ReleaseCountInfo(struct SCountInfo * &countInfo)
    {
         s_countinfo_slab.ReleaseNode(countInfo);
    }

    bool CPageMgr::TopDownMMap()
    {   
        return (m_iFlags & (1 << E_MMAP_DIRECTION_BIT));
    }

    size_t CPageMgr::Hash(size_t pagecount)
    {
        return (pagecount) & (E_HASH_SIZE - 1);
    }

    struct CPageMgr::SHashNode * CPageMgr::GetHashNode(size_t pagecount)
    {
        size_t bucket = Hash(pagecount);
        return &m_cHash[bucket];
    }

    void * CPageMgr::HashTreeGetObject(void * node)
    {
        return (void *)rb_entry((rb_node *)node, struct SIndexInfo, hash_node);
    }
    
    rb_node * CPageMgr::HashTreeGetRbNode(void * object)
    {
        return &(((struct SIndexInfo *)object)->hash_node);
    }
    
    size_t CPageMgr::HashTreeSearch(const void * lhs, const void * rhs)
    {
        struct SIndexInfo * lInfo = (struct SIndexInfo *)lhs;
        size_t & pagecount = *(size_t *)rhs;

        return lInfo->page_count - pagecount;
    }
    
    size_t CPageMgr::HashTreeInsert(const void * lhs, const void * rhs)
    {
        struct SIndexInfo * lInfo = (struct SIndexInfo *)lhs;
        struct SIndexInfo * rInfo = (struct SIndexInfo *)rhs;

        return lInfo->page_count - rInfo->page_count;
    }

    void * CPageMgr::AddressTreeGetObject(void * node)
    {
        return (void *)rb_entry((rb_node *)node, struct SPageInfo, address_node);
    }
    
    rb_node * CPageMgr::AddressTreeGetRbNode(void * object)
    {
        return &(((struct SPageInfo *)object)->address_node);
    }
    
    size_t CPageMgr::AddressTreeSearch(const void * lhs, const void * rhs)
    {
        struct SPageInfo * lInfo = (struct SPageInfo *)lhs;
        size_t start = lInfo->base_address;
        size_t end   = lInfo->base_address + (lInfo->page_count << FT_PAGE_BIT);
        size_t & address = *(size_t *)rhs;

        if (start <= address && address < end) {
            return 0;
        } else if (address >= end) {
            return 1;
        } else {
            return -1;
        }
    }
    
    size_t CPageMgr::AddressTreeInsert(const void * lhs, const void * rhs)
    {
        struct SPageInfo * lInfo = (struct SPageInfo *)lhs;
        struct SPageInfo * rInfo = (struct SPageInfo *)rhs;

        return lInfo->base_address - rInfo->base_address;
    }

    void * CPageMgr::CountTreeGetObject(void * node)
    {
        return (void *)rb_entry((rb_node *)node, struct SCountInfo, count_node);
    }
    
    rb_node * CPageMgr::CountTreeGetRbNode(void * object)
    {
        return &(((struct SCountInfo *)object)->count_node);
    }
    
    size_t CPageMgr::CountTreeSearch(const void * lhs, const void * rhs)
    {
        struct SCountInfo * lInfo = (struct SCountInfo *)lhs;
        size_t & pagecount = *(size_t *)rhs;

        return lInfo->page_count - pagecount;
    }
    
    size_t CPageMgr::CountTreeInsert(const void * lhs, const void * rhs)
    {
        struct SCountInfo * lInfo = (struct SCountInfo *)lhs;
        struct SCountInfo * rInfo = (struct SCountInfo *)rhs;

        return lInfo->page_count - rInfo->page_count;
    }
    
    void * CPageMgr::FreeTreeGetObject(void * node)
    {
        return (void *)rb_entry((rb_node *)node, struct SPageInfo, free_node);
    }
    
    rb_node * CPageMgr::FreeTreeGetRbNode(void * object)
    {
        return &(((struct SPageInfo *)object)->free_node);
    }
    
    size_t CPageMgr::FreeTreeSearch(const void * lhs, const void * rhs)
    {
        struct SPageInfo * lInfo = (struct SPageInfo *)lhs;
        struct SPageInfo * rInfo = (struct SPageInfo *)rhs;

        return lInfo->base_address - rInfo->base_address;
    }
    
    size_t CPageMgr::FreeTreeInsert(const void * lhs, const void * rhs)
    {
        struct SPageInfo * lInfo = (struct SPageInfo *)lhs;
        struct SPageInfo * rInfo = (struct SPageInfo *)rhs;

        return lInfo->base_address - rInfo->base_address;
    }
    
    
    void * CPageMgr::RbSearch(struct rb_root *root, 
        void * object, RbGetObjectFunc getObject, RbSearchFunc search)
    {
        struct rb_node *node = root->rb_node;

        while (node) {
            void * datanode = (this->*getObject)(node);
            size_t result = (this->*search)(datanode, object);

            if (result < 0)
                node = node->rb_left;
            else if (result > 0)
                node = node->rb_right;
            else
                return datanode;
        }
        return NULL;
    }
    
    int CPageMgr::RbInsert(struct rb_root *root, void *data, 
        RbGetObjectFunc getObject, RbGetNodeFunc getNode, RbInsertFunc compare)
    {
        struct rb_node **newnode = &(root->rb_node), *parent = NULL;

        /* Figure out where to put new node */
        while (*newnode) {
            void * thisnode = (this->*getObject)(*newnode);
            size_t result = (this->*compare)(thisnode, data);

            parent = *newnode;
            if (result < 0)
                newnode = &((*newnode)->rb_left);
            else if (result > 0)
                newnode = &((*newnode)->rb_right);
            else
                return 0;
        }

        /* Add new node and rebalance tree. */
        rb_link_node((this->*getNode)(data), parent, newnode);
        rb_insert_color((this->*getNode)(data), root);

        return 1;
    }

    void CPageMgr::RbRemove(rb_root * root, void * object, 
        RbGetNodeFunc getNode)
    {
        rb_erase((this->*getNode)(object), root);
    }
}
