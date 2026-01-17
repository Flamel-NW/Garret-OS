#ifndef __KERNEL_MM_PMM_H__
#define __KERNEL_MM_PMM_H__

#include "assert.h"
#include "defs.h"
#include "memlayout.h"
#include "page.h"
#include "errno.h"


// pages (include kernel-reserved and free)
extern struct page* g_pages;

// amount of kernel-reserved pages
extern u64 g_num_qemu_pages;

// amount of kernel-reserved pages
extern u64 g_num_kernel_pages;

// amount of free pages
extern u64 g_num_free_pages;

extern pte_t* g_bpt;

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
    void (*init_memmap) (struct page* base, u64 n);

    // allocate >= n pages, depend on the allocation algorithm
    struct page* (*alloc_pages) (u64 n);

    // free >= pages, with "base" addr of page description structures(memlayout.h)
    void (*free_pages) (struct page* base, u64 n);

    // count the number of free pages
    u64 (*count_free_pages) ();

    // test the correctness of xxx_pm_manager
    void (*test) ();
};


// init_pmm - initialize the physical memory management
void init_pmm();

// alloc_pages - call pmm->alloc_pages to allocate a continuous n * PAGE_SIZE memory
struct page* alloc_pages(u64 n);

static inline struct page* alloc_page() {
    return alloc_pages(1);
}

// free_pages - call pmm->free_pages to free continuous n * PAGE_SIZE memory
void free_pages(struct page* base, u64 n);

static inline void free_page(struct page* base) {
    free_pages(base, 1);
}

// count_free_pages - call pmm->count_free_pages to get the size (num * PAGE_SIZE) of current free memory
u64 count_free_pages();

struct free_area* get_free_area();

static inline ppn_t page2ppn(struct page* page) {
    ppn_t ppn = page - g_pages;
    ASSERT(ppn >= g_num_qemu_pages + g_num_kernel_pages && 
        ppn < g_num_qemu_pages + g_num_kernel_pages + g_num_free_pages);
    return ppn;
}

static inline u64 page2pa(struct page* page) {
    return page2ppn(page) << PAGE_SHIFT;
}

static inline u64 page2va(struct page* page) {
    return pa2va(page2pa(page));
}

static inline struct page* pa2page(u64 pa) {
    ppn_t ppn = pa >> PAGE_SHIFT;
    ASSERT(ppn >= g_num_qemu_pages + g_num_kernel_pages && 
        ppn < g_num_qemu_pages + g_num_kernel_pages + g_num_free_pages);
    return &g_pages[ppn];
}

static inline struct page* va2page(u64 va) {
    return pa2page(va2pa(va));
}

static inline pte_t ppn2pte(ppn_t ppn, u32 flags) {
    return (ppn << PTE_SHIFT) | flags;
}

static inline ppn_t pte2ppn(pte_t pte) {
    return (u64) pte >> PTE_SHIFT;
}

static inline struct page* pte2page(pte_t pte) {
    ASSERT(pte & PTE_V);
    return &g_pages[pte2ppn(pte)];
}

static inline u64 pte2va(pte_t pte) {
    return page2va(pte2page(pte));
}

// get_pte - get pte and return the kernel virtual address of this pte for addr
//      if the PT contains this pte didn't exist, alloc a page for PT
pte_t* get_pte(pte_t* p_gpt, u64 addr, bool need_create);

// add_page - build a map of pa of a page with the addr 
i32 add_page(pte_t* p_gpt, struct page* page, u64 addr, u32 flags);

// del_page - free an page struct which is related addr and clean(invalidate) pte which is related addr
void del_page(pte_t* p_gpt, u64 addr);

void* malloc(u64 n);
void free(void* ptr, u64 n);

// alloc_page_gpte - allocate a page size memory & setup an addr map in a gigapage
struct page* alloc_page_gpte(pte_t* p_gpt, u64 addr, u32 flags);

void del_range(pte_t* p_gpt, u64 start, u64 end);
void free_range(pte_t* p_gpt, u64 start, u64 end);

// copy_range - copy content of memory (start, end) of one process A to another process B
//              CALL GRAPH: copy_vmm -> dup_vmm -> copy_range
i32 copy_range(pte_t* from, pte_t* to, u64 start, u64 end);

// va2pa_mmu - Simulate RISC-V MMU address translation process with debug output
// This function walks through the 3-level page table (Sv39) to translate
// a virtual address to a physical address, exactly as the MMU hardware does.
// It prints detailed information about each step of the translation process.
//
// @p_gpt: pointer to the Gigapage Table (root of page table)
// @va: virtual address to translate
// @return: physical address if translation succeeds, 0 if translation fails
u64 va2pa_mmu(pte_t* p_gpt, u64 va);

#endif // __KERNEL_MM_PMM_H__
