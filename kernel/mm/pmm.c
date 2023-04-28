#include "pmm.h"

#include "assert.h"
#include "defs.h"
#include "errno.h"
#include "best_fit_pmm.h"
#include "memlayout.h"
#include "memlayout.h"
#include "page.h"
#include "stdio.h"
#include "intr.h"
#include "stdlib.h"
#include "string.h"
#include "swap.h"
#include "vmm.h"


// pages (include kernel-reserved and free)
struct page* g_pages;

// amount of kernel-reserved pages
size_t g_num_qemu_pages = DRAM_BASE_PA / PAGE_SIZE;

// amount of kernel-reserved pages
size_t g_num_kernel_pages;

// amount of free pages
size_t g_num_free_pages = (FREE_END_VA - FREE_BEGIN_VA) / PAGE_SIZE;

// physical memory management
static const struct pm_manager* g_pm_manager;

pte_t* g_satp;

static void check_page();

// init_pm_manager - initialize a pm_manager instance
static void init_pm_manager() {
    g_pm_manager = &g_best_fit_pm_manager;
    putstr("physical memory menager: "); putstr(g_pm_manager->name); putch('\n');
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
    while (1) {
        bool intr_flag = local_intr_save();
        {
            page = g_pm_manager->alloc_pages(n);
        }
        local_intr_restore(intr_flag);

        if (page || !g_swap_init_flag)
            break;
        
        //页不够了 就要换出点空
        swap_out(g_check_vmm, n);
    }
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

struct free_area* get_free_area() {
    return g_pm_manager->free_area;
}

static void init_page() {
    extern char end[];

    g_pages = (struct page*) roundup((size_t) end, PAGE_SIZE);

    g_num_kernel_pages = (KERNEL_END_VA - DRAM_BASE_VA) / PAGE_SIZE;
    for (size_t i = 0; i < g_num_qemu_pages + g_num_kernel_pages; i++)
        g_pages[i].flags |= PAGE_RESERVED;

    init_memmap(&g_pages[g_num_qemu_pages + g_num_kernel_pages], g_num_free_pages);
}

// get_page - get related page struct for addr using a gpte
struct page* get_page(pte_t* p_gpte, uintptr_t addr, pte_t** pp_pte) {
    pte_t* p_pte = get_pte(p_gpte, addr, 0);
    if (p_pte)
        *pp_pte = p_pte;
    if (p_pte && *p_pte & PTE_V)
        return pte2page(*p_pte);
    else
        return NULL;
}

// init_pmm - initialize the physical memory management
void init_pmm() {
    // We need to alloc/free the physical memory (granularity is 4KB or other size).
    // So a framework of physical memory manager (struct pm_manager) is defined in pmm.h
    
    // First we should init a physical memory manager(pmm) based on the framework.
    // Then pmm can alloc/free the physical memory.
    // Now the best_fit/best_fit/worst_fit/buddy_system pmm are available.
    init_pm_manager();
    
    // detect physical memory space, reserve already used memory
    // then use pmm->init_memmap to create free page list
    init_page();

    extern char boot_page_table_sv39[];
    g_satp = (pte_t*) boot_page_table_sv39;

    // use pmm->check to verify the correctness of the alloc/free function in a pmm
    g_pm_manager->check();
    putstr("pmm check succeed!\n\n");

    check_page();
}

// get_pte - get pte and return the kernel virtual address of this pte for addr
//      if the PT contains this pte didn't exist, alloc a page for PT
pte_t* get_pte(pte_t* p_tpte, uintptr_t addr, bool init) {
    pte_t* p_gpte = &p_tpte[get_gptx(addr)];
    if (!(*p_gpte & PTE_V)) {
        struct page* page;
        if (!init || !(page = alloc_page()))
            return NULL;
        page->ref = 1;
        memset((void*) page2va(page), 0, PAGE_SIZE);
        *p_gpte = ppn2pte(page2ppn(page), PTE_V);
    }

    pte_t* p_mpte = &((pte_t*) pte2addr(*p_gpte))[get_mptx(addr)];
    if (!(*p_mpte & PTE_V)) {
        struct page* page;
        if (!init || !(page = alloc_page()))
            return NULL;
        page->ref = 1;
        memset((void*) page2va(page), 0, PAGE_SIZE);
        *p_mpte = ppn2pte(page2ppn(page), PTE_V);
    }
    return &((pte_t*) pte2addr(*p_mpte))[get_ptx(addr)];
}

// add_page - build a map of pa of a page with the addr 
int32_t add_page(pte_t* p_gpte, struct page* page, uintptr_t addr, uint32_t flags) {
    pte_t* p_pte = get_pte(p_gpte, addr, 1);
    if (!p_pte)
        return ENOMEM;
    page->ref++;
    if (*p_pte & PTE_V) {
        struct page* p = pte2page(*p_pte);
        if (p == page)
            page->ref--;
        else 
            del_page(p_gpte, addr);
    }
    *p_pte = ppn2pte(page2ppn(page), PTE_V | flags);
    flush_tlb();
    return 0;
}

// del_page - free an page struct which is related addr and clean(invalidate) pte which is related addr
void del_page(pte_t* p_gpte, uintptr_t addr) {
    pte_t* p_pte = get_pte(p_gpte, addr, 0);
    if (p_pte && *p_pte & PTE_V) {
        struct page* page = pte2page(*p_pte);
        page->ref--;
        if (!page->ref) 
            free_page(page);
        *p_pte = 0;
        flush_tlb();
    }
}

// alloc_page_gpte - allocate a page size memory & setup an addr map in a gigapage
struct page* alloc_page_gpte(pte_t* p_gpte, uintptr_t addr, uint32_t flags) {
    struct page* page = alloc_page();
    if (page) {
        if (add_page(p_gpte, page, addr, flags)) {
            free_page(page);
            return NULL;
        }
        if (g_swap_init_flag) {
            swap_set_swappable(g_check_vmm, page);
            page->pra_addr = addr;
            ASSERT(page->ref == 1);
        }
    }
    return page;
}

void* malloc(size_t n) {
    size_t num_pages = (roundup(n, PAGE_SIZE)) >> PAGE_SHIFT;
    ASSERT(num_pages > 0 && num_pages < g_num_free_pages);
    struct page* page = alloc_pages(num_pages);
    return (void*) page2va(page);
}

void free(void* ptr, size_t n) {
    size_t num_pages = (roundup(n, PAGE_SIZE)) >> PAGE_SHIFT;
    ASSERT(num_pages > 0 && num_pages < g_num_free_pages);
    free_pages(va2page((uintptr_t) ptr), num_pages);
}

static void check_page() {
    size_t num_free_pages = count_free_pages();

    ASSERT(g_satp != NULL && get_offset((uintptr_t) g_satp) == 0);
    ASSERT(get_page(g_satp, 0, NULL) == NULL);

    struct page* p1;
    struct page* p2;
    p1 = alloc_page();
    ASSERT(add_page(g_satp, p1, 0, 0) == 0);

    pte_t* p_pte = get_pte(g_satp, 0x0, 0);
    ASSERT(p_pte != NULL);
    ASSERT(pte2page(*p_pte) == p1);
    ASSERT(p1->ref == 1);

    p_pte = (pte_t*) pte2addr(g_satp[0]);
    p_pte = (pte_t*) pte2addr(p_pte[0]) + 1;
    ASSERT(get_pte(g_satp, PAGE_SIZE, 0) == p_pte);

    p2 = alloc_page();
    ASSERT(add_page(g_satp, p2, PAGE_SIZE, PTE_W | PTE_V) == 0);
    ASSERT((p_pte = get_pte(g_satp, PAGE_SIZE, 0)) != NULL);
    ASSERT((*p_pte & PTE_W) != 0);
    ASSERT(p2->ref == 1);

    ASSERT(add_page(g_satp, p1, PAGE_SIZE, 0) == 0);
    ASSERT(p1->ref == 2);
    ASSERT(p2->ref == 0);
    p_pte = get_pte(g_satp, PAGE_SIZE, 0);
    ASSERT(p_pte != NULL);
    ASSERT(pte2page(*p_pte) == p1);

    del_page(g_satp, 0);
    ASSERT(p1->ref == 1);
    ASSERT(p2->ref == 0);

    del_page(g_satp, PAGE_SIZE);
    ASSERT(p1->ref == 0);
    ASSERT(p2->ref == 0);

    ASSERT(pte2page(g_satp[0])->ref == 1);

    pte_t* p_gpte1 = g_satp;
    pte_t* p_gpte2 = (pte_t*) page2va(pte2page(g_satp[0]));

    free_page(pte2page(p_gpte1[0]));
    free_page(pte2page(p_gpte2[0]));
    g_satp[0] = 0;

    ASSERT(num_free_pages == count_free_pages());

    num_free_pages = count_free_pages();

    for (uintptr_t i = rounddown(KERNEL_BEGIN_VA, PAGE_SIZE); 
            i < g_num_kernel_pages << PAGE_SHIFT; i += PAGE_SIZE) {
        ASSERT((p_pte = get_pte(g_satp, i, 0)) != NULL);
        ASSERT(pte2addr(*p_pte) == i);
    }

    ASSERT(g_satp[0] == 0);

    struct page* p = alloc_page();
    ASSERT(add_page(g_satp, p, 0x100, PTE_W | PTE_R | PTE_V) == 0);
    ASSERT(p->ref == 1);
    ASSERT(add_page(g_satp, p, 0x100 + PAGE_SIZE, PTE_W | PTE_R | PTE_V) == 0);
    ASSERT(p->ref == 2);

    pte_t* t_pte1 = get_pte(g_satp, 0x100, 0);
    pte_t* t_pte2 = get_pte(g_satp, 0x100 + PAGE_SIZE, 0);

    ppn_t t_ppn1 = pte2ppn(*t_pte1);
    ppn_t t_ppn2 = pte2ppn(*t_pte2);

    uintptr_t t_addr1 = pte2addr(*t_pte1);
    uintptr_t t_addr2 = pte2addr(*t_pte2);

    const char *str = "ucore: Hello world!!";
    strcpy((void *)0x100, str);
    ASSERT(strcmp((void *)0x100, (void *)(0x100 + PAGE_SIZE)) == 0);

    *(char *)(page2va(p) + 0x100) = '\0';
    ASSERT(strlen((const char *)0x100) == 0);

    p_gpte1 = g_satp;
    p_gpte2 = (pte_t*) page2va(pte2page(g_satp[0]));
    free_page(p);
    free_page(pte2page(p_gpte1[0]));
    free_page(pte2page(p_gpte2[0]));
    g_satp[0] = 0;

    ASSERT(num_free_pages == count_free_pages());

    putstr("page check succeed!\n\n");
}
