#ifndef __KERNEL_MM_PMM_H__
#define __KERNEL_MM_PMM_H__

#include "defs.h"
#include "memlayout.h"


// pm_manager is a physical memory management class. 
// A special pmm manager - xxx_pm_manager
// only needs to implement the methods in pm_manager class,
// then xxx_pm_manager can be used to manage the total physical memory space
struct pm_manager {
    const char* name;   // xxx_pm_manager's name

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

// virutal address of kernel-reserved page array
extern struct page* g_kernel_pages;

// amount of kernel-reserved pages
extern size_t g_num_kernel_pages;

// physical memory management
extern const struct pm_manager* g_pm_manager;


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



#endif // __KERNEL_MM_PMM_H__
