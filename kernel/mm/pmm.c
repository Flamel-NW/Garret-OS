#include "pmm.h"

#include "defs.h"
#include "first_fit_pmm.h"
#include "memlayout.h"
#include "mmu.h"
#include "stdio.h"
#include "intr.h"
#include "stdlib.h"


// virutal address of kernel-reserved page array
struct page* g_kernel_pages;

// amount of kernel-reserved pages
size_t g_num_kernel_pages = 0;

// physical memory management
const struct pm_manager* g_pm_manager;

// init_pm_manager - initialize a pm_manager instance
static void init_pm_manager() {
    g_pm_manager = &g_first_fit_pm_manager;
    putstr("memory menagement: "); putstr(g_pm_manager->name); putch('\n');
    putch('\n');
    g_pm_manager->init();
}

// init_memmap - call pmm->init_memmp to build page struct for free memory
static void init_memmap(struct page* base, size_t n) {
    g_pm_manager->init_memmap(base, n);
}

// alloc_pages - call pmm->alloc_pages to allocate a continuous n * PAGE_SIZE memory
struct page* alloc_pages(size_t n) {
    struct page* page = NULL;
    bool intr_flag = local_intr_save();
    {
        page = g_pm_manager->alloc_pages(n);
    }
    local_intr_restore(intr_flag);
    return page;
}

// free_pages - call pmm->free_pages to free continuous n * PAGE_SIZE memory
void free_pages(struct page* base, size_t n) {
    bool intr_flag = local_intr_save();
    {
        g_pm_manager->free_pages(base, n);
    }
    local_intr_restore(intr_flag);
}

// count_free_pages - call pmm->count_free_pages to get the size (num * PAGE_SIZE) of current free memory
size_t count_free_pages() {
    size_t ret;
    bool intr_flag = local_intr_save();
    {
        ret = g_pm_manager->count_free_pages();
    }
    local_intr_restore(intr_flag);
    return ret;
}

static void init_page() {
    extern char end[];

    g_kernel_pages = (struct page*) ROUNDUP((void*) end, PAGE_SIZE);

    g_num_kernel_pages = (KERNEL_END_VA - (size_t) g_kernel_pages) / PAGE_SIZE;
    for (size_t i = 0; i < g_num_kernel_pages; i++)
        set_page_reserved(&g_kernel_pages[i]);

    init_memmap((struct page*) FREE_BEGIN_VA, FREE_PAGES);
}

static void check() {
    g_pm_manager->check();
    putstr("pmm check succeed!\n\n");
}

// init_pmm - initialize the physical memory management
void init_pmm() {
    // We need to alloc/free the physical memory (granularity is 4KB or other size).
    // So a framework of physical memory manager (struct pm_manager) is defined in pmm.h
    
    // First we should init a physical memory manager(pmm) based on the framework.
    // Then pmm can alloc/free the physical memory.
    // Now the first_fit/best_fit/worst_fit/buddy_system pmm are available.
    init_pm_manager();
    
    // detect physical memory space, reserve already used memory
    // then use pmm->init_memmap to create free page list
    init_page();

    // use pmm->check to verify the correctness of the alloc/free function in a pmm
    check();
}
