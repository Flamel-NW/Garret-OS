#ifndef __KERNEL_MM_PMM_H__
#define __KERNEL_MM_PMM_H__

#include "assert.h"
#include "defs.h"
#include "memlayout.h"
#include "page.h"


// pages (include kernel-reserved and free)
extern struct page* g_pages;

// amount of kernel-reserved pages
extern size_t g_num_qemu_pages;

// amount of kernel-reserved pages
extern size_t g_num_kernel_pages;

// amount of free pages
extern size_t g_num_free_pages;

extern pte_t* g_satp;

// pm_manager is a physical memory management class. 
// A special pmm manager - xxx_pm_manager
// only needs to implement the methods in pm_manager class,
// then xxx_pm_manager can be used to manage the total physical memory space
struct pm_manager {
    const char* name;   // xxx_pm_manager's name
    struct free_area* free_area;

    // initialize internal description & management data structure
    // (free block list, number of free block) of xxx_pm_manager
    void (*init) ();

    // setup description & management data structure according to
    // the initial free physical memory space
    void (*init_memmap) (struct page* base, size_t n);

    // allocate >= n pages, depend on the allocation algorithm
    struct page* (*alloc_pages) (size_t n);

    // free >= pages, with "base" addr of page description structures(memlayout.h)
    void (*free_pages) (struct page* base, size_t n);

    // count the number of free pages
    size_t (*count_free_pages) ();

    // check the correctness of xxx_pm_manager
    void (*check) ();
};


// init_pmm - initialize the physical memory management
void init_pmm();

// alloc_pages - call pmm->alloc_pages to allocate a continuous n * PAGE_SIZE memory
struct page* alloc_pages(size_t n);

static inline struct page* alloc_page() {
    return alloc_pages(1);
}

// free_pages - call pmm->free_pages to free continuous n * PAGE_SIZE memory
void free_pages(struct page* base, size_t n);

static inline void free_page(struct page* base) {
    free_pages(base, 1);
}

// count_free_pages - call pmm->count_free_pages to get the size (num * PAGE_SIZE) of current free memory
size_t count_free_pages();

struct free_area* get_free_area();

static inline ppn_t page2ppn(struct page* page) {
    ppn_t ppn = page - g_pages;
    ASSERT(ppn >= g_num_qemu_pages + g_num_kernel_pages && 
        ppn < g_num_qemu_pages + g_num_kernel_pages + g_num_free_pages);
    return ppn;
}

static inline uintptr_t page2pa(struct page* page) {
    return page2ppn(page) << PAGE_SHIFT;
}

static inline uintptr_t page2va(struct page* page) {
    return pa2va(page2ppn(page) << PAGE_SHIFT);
}

static inline struct page* pa2page(uintptr_t pa) {
    ppn_t ppn = pa >> PTX_SHIFT;
    ASSERT(ppn >= g_num_qemu_pages + g_num_kernel_pages && 
        ppn < g_num_qemu_pages + g_num_kernel_pages + g_num_free_pages);
    return &g_pages[ppn];
}

static inline struct page* va2page(uintptr_t va) {
    return pa2page(va2pa(va));
}

static inline pte_t ppn2pte(ppn_t ppn, uint32_t flags) {
    return (ppn << PTE_SHIFT) | flags;
}

static inline ppn_t pte2ppn(pte_t pte) {
    return (uintptr_t) pte >> PTE_SHIFT;
}

static inline struct page* pte2page(pte_t pte) {
    ASSERT(pte & PTE_V);
    return &g_pages[pte2ppn(pte)];
}

static inline uintptr_t pte2addr(pte_t pte) {
    return page2va(pte2page(pte));
}

// get_pte - get pte and return the kernel virtual address of this pte for addr
//      if the PT contains this pte didn't exist, alloc a page for PT
pte_t* get_pte(pte_t* p_tpte, uintptr_t addr, bool init);

// add_page - build a map of pa of a page with the addr 
int32_t add_page(pte_t* p_gpte, struct page* page, uintptr_t addr, uint32_t flags);

// del_page - free an page struct which is related addr and clean(invalidate) pte which is related addr
void del_page(pte_t* p_gpte, uintptr_t addr);

void* malloc(size_t n);
void free(void* ptr, size_t n);

// alloc_page_gpte - allocate a page size memory & setup an addr map in a gigapage
struct page* alloc_page_gpte(pte_t* p_gpte, uintptr_t addr, uint32_t flags);

#endif // __KERNEL_MM_PMM_H__
