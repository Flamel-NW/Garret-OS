#include "pmm.h"

#include "assert.h"
#include "defs.h"
#include "errno.h"
#include "best_fit_pmm.h"
#include "memlayout.h"
#include "page.h"
#include "stdio.h"
#include "sync.h"
#include "stdlib.h"
#include "string.h"
#include "swap.h"
#include "vmm.h"


// pages (include kernel-reserved and free)
struct page* g_pages;

// amount of kernel-reserved pages
u64 g_num_qemu_pages = DRAM_BEGIN_PA / PAGE_SIZE;

// amount of kernel-reserved pages
u64 g_num_kernel_pages;

// amount of free pages
u64 g_num_free_pages = (USER_END - USER_BEGIN) / PAGE_SIZE;

// physical memory management
static const struct pm_manager* g_pm_manager;

// boot page table
pte_t* g_bpt;

static void test_page();

// init_pm_manager - initialize a pm_manager instance
static void init_pm_manager() {
    g_pm_manager = &g_best_fit_pm_manager;
    putstr("physical memory menager: "); putstr(g_pm_manager->name); putch('\n');
    putch('\n');
    g_pm_manager->init();
}

// init_memmap - call pmm->init_memmp to build page struct for free memory
static void init_memmap(struct page* base, u64 n) {
    g_pm_manager->init_memmap(base, n);
}

// alloc_pages - call pmm->alloc_pages to allocate a continuous n * PAGE_SIZE memory
struct page* alloc_pages(u64 n) {
    struct page* page = NULL;
    while (true) {
        bool intr_flag = local_intr_save();
        {
            page = g_pm_manager->alloc_pages(n);
        }
        local_intr_restore(intr_flag);

        if (page || !g_swap_init_flag)
            break;
        
        //页不够了 就要换出点空
        swap_out(g_test_vmm, n);
    }
   return page;
}

// free_pages - call pmm->free_pages to free continuous n * PAGE_SIZE memory
void free_pages(struct page* base, u64 n) {
    bool intr_flag = local_intr_save();
    {
        g_pm_manager->free_pages(base, n);
    }
    local_intr_restore(intr_flag);
}

// count_free_pages - call pmm->count_free_pages to get the size (num * PAGE_SIZE) of current free memory
u64 count_free_pages() {
    u64 ret;
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
    extern u8 end[];

    g_pages = (struct page*) roundup((u64) end, PAGE_SIZE);

    g_num_kernel_pages = (KERNEL_END_VA - DRAM_BEGIN_VA) / PAGE_SIZE;
    for (u64 i = 0; i < g_num_qemu_pages + g_num_kernel_pages; i++)
        g_pages[i].flags |= PAGE_RESERVED;

    init_memmap(&g_pages[g_num_qemu_pages + g_num_kernel_pages], g_num_free_pages);
}

// get_page - get related page struct for addr using a gpte
struct page* get_page(pte_t* p_gpt, u64 addr, pte_t** pp_pte) {
    pte_t* p_pte = get_pte(p_gpt, addr, 0);
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

    extern u8 boot_page_table_sv39[];
    g_bpt = (pte_t*) boot_page_table_sv39;

    // use pmm->test to verify the correctness of the alloc/free function in a pmm
    g_pm_manager->test();
    putstr("pmm test succeed!\n\n");

    test_page();
}

// get_pte - get pte and return the kernel virtual address of this pte for addr
//           if the PT contains this pte didn't exist, alloc a page for PT
pte_t* get_pte(pte_t* p_gpt, u64 addr, bool need_create) {
    pte_t* p_gpte = &p_gpt[get_gptx(addr)];
    if (!(*p_gpte & PTE_V)) {
        struct page* page;
        if (!need_create || !(page = alloc_page()))
            return NULL;
        page->ref = 1;
        memset((void*) page2va(page), 0, PAGE_SIZE);
        *p_gpte = ppn2pte(page2ppn(page), PTE_V);
    }

    pte_t* p_mpte = &((pte_t*) pte2va(*p_gpte))[get_mptx(addr)];
    if (!(*p_mpte & PTE_V)) {
        struct page* page;
        if (!need_create || !(page = alloc_page()))
            return NULL;
        page->ref = 1;
        memset((void*) page2va(page), 0, PAGE_SIZE);
        *p_mpte = ppn2pte(page2ppn(page), PTE_V);
    }
    return &((pte_t*) pte2va(*p_mpte))[get_ptx(addr)];
}

void del_range(pte_t* p_gpt, u64 start, u64 end) {
    ASSERT(start % PAGE_SIZE == 0 && end % PAGE_SIZE == 0);
    ASSERT(user_access(start, end));

    do {
        pte_t* p_pte = get_pte(p_gpt, start, 0);
        if (!p_pte) {
            start = rounddown(start + PT_SIZE, PT_SIZE);
            continue;
        }
        if (*p_pte)
            del_page(p_gpt, start);
        start += PAGE_SIZE;
    } while(start != 0 && start < end);
}

void free_range(pte_t* p_gpt, u64 start, u64 end) {
    ASSERT(start % PAGE_SIZE == 0 && end % PAGE_SIZE == 0);
    ASSERT(user_access(start, end));

    start = rounddown(start, PAGE_SIZE);
    do {
        i32 index = get_gptx(start);
        if (p_gpt[index] & PTE_V) {
            free_page(pte2page(p_gpt[index]));
            p_gpt[index] = 0;
        }
        start += PT_SIZE;
    } while (start != 0 && start < end);
}

// copy_range - copy content of memory (start, end) of one process A to another process B
//              CALL GRAPH: copy_vmm -> dup_vmm -> copy_range
i32 copy_range(pte_t* from, pte_t* to, u64 start, u64 end) {
    ASSERT(start % PAGE_SIZE == 0 && end % PAGE_SIZE == 0);
    ASSERT(user_access(start, end));

    // copy content by page unit.
    do {
        // call get_pte to find process A's pte according to the addr start
        pte_t* p_pte = get_pte(from, start, 0);
        if (!p_pte) {
            start = rounddown(start + PT_SIZE, PT_SIZE);
            continue;
        }

        // call get_pte to find process B's pte according to the addr start. If pte is NULL, just alloc a PT
        if (*p_pte & PTE_V) {
            if (!get_pte(to, start, 1)) // TODO
                return -E_NO_MEM;
            u32 flags = *p_pte & PTE_USER;
            struct page* page = pte2page(*p_pte);       // get page from p_pte
            struct page* new_page = alloc_page();       // alloc page for process B
            
            ASSERT(page != NULL);
            ASSERT(new_page != NULL);

            memcpy((void*) page2va(new_page), (void*) page2va(page), PAGE_SIZE);

            i32 ret = add_page(to, new_page, start, flags);
            if (ret)
                return ret;
        }

        start += PAGE_SIZE;
    } while (start && start < end);

    return 0;
}

// add_page - build a map of pa of a page with the addr 
i32 add_page(pte_t* p_gpt, struct page* page, u64 addr, u32 flags) {
    pte_t* p_pte = get_pte(p_gpt, addr, 1);
    if (!p_pte)
        return -E_NO_MEM;
    page->ref++;
    if (*p_pte & PTE_V) {
        struct page* p = pte2page(*p_pte);
        if (p == page)
            page->ref--;
        else 
            del_page(p_gpt, addr);
    }
    *p_pte = ppn2pte(page2ppn(page), PTE_V | flags);
    flush_tlb();
    return 0;
}

// del_page - free an page struct which is related addr and clean(invalidate) pte which is related addr
void del_page(pte_t* p_gpt, u64 addr) {
    pte_t* p_pte = get_pte(p_gpt, addr, 0);
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
struct page* alloc_page_gpte(pte_t* p_gpt, u64 addr, u32 flags) {
    struct page* page = alloc_page();
    if (page) {
        if (add_page(p_gpt, page, addr, flags)) {
            free_page(page);
            return NULL;
        }
        if (g_swap_init_flag && g_test_vmm) {
            swap_set_swappable(g_test_vmm, page);
            page->pra_addr = addr;
            ASSERT(page->ref == 1);
        }
    }
    return page;
}

void* malloc(u64 n) {
    u64 num_pages = (roundup(n, PAGE_SIZE)) >> PAGE_SHIFT;
    ASSERT(num_pages > 0 && num_pages < g_num_free_pages);
    struct page* page = alloc_pages(num_pages);
    return (void*) page2va(page);
}

void free(void* ptr, u64 n) {
    u64 num_pages = (roundup(n, PAGE_SIZE)) >> PAGE_SHIFT;
    ASSERT(num_pages > 0 && num_pages < g_num_free_pages);
    free_pages(va2page((u64) ptr), num_pages);
}

static void test_page() {
    u64 num_free_pages = count_free_pages();

    ASSERT(g_bpt != NULL && get_offset((u64) g_bpt) == 0);
    ASSERT(get_page(g_bpt, 0, NULL) == NULL);

    struct page* p1;
    struct page* p2;
    p1 = alloc_page();
    ASSERT(add_page(g_bpt, p1, 0, 0) == 0);

    pte_t* p_pte = get_pte(g_bpt, 0x0, 0);
    ASSERT(p_pte != NULL);
    ASSERT(pte2page(*p_pte) == p1);
    ASSERT(p1->ref == 1);

    p_pte = (pte_t*) pte2va(g_bpt[0]);
    p_pte = (pte_t*) pte2va(p_pte[0]) + 1;
    ASSERT(get_pte(g_bpt, PAGE_SIZE, 0) == p_pte);

    p2 = alloc_page();
    ASSERT(add_page(g_bpt, p2, PAGE_SIZE, PTE_W | PTE_V) == 0);
    ASSERT((p_pte = get_pte(g_bpt, PAGE_SIZE, 0)) != NULL);
    ASSERT((*p_pte & PTE_W) != 0);
    ASSERT(p2->ref == 1);

    ASSERT(add_page(g_bpt, p1, PAGE_SIZE, 0) == 0);
    ASSERT(p1->ref == 2);
    ASSERT(p2->ref == 0);
    p_pte = get_pte(g_bpt, PAGE_SIZE, 0);
    ASSERT(p_pte != NULL);
    ASSERT(pte2page(*p_pte) == p1);

    del_page(g_bpt, 0);
    ASSERT(p1->ref == 1);
    ASSERT(p2->ref == 0);

    del_page(g_bpt, PAGE_SIZE);
    ASSERT(p1->ref == 0);
    ASSERT(p2->ref == 0);

    ASSERT(pte2page(g_bpt[0])->ref == 1);

    pte_t* p_gpt1 = g_bpt;
    pte_t* p_gpt2 = (pte_t*) page2va(pte2page(g_bpt[0]));

    free_page(pte2page(p_gpt1[0]));
    free_page(pte2page(p_gpt2[0]));
    g_bpt[0] = 0;

    ASSERT(num_free_pages == count_free_pages());

    num_free_pages = count_free_pages();

    for (u64 i = rounddown(KERNEL_BEGIN_VA, PAGE_SIZE); 
            i < g_num_kernel_pages << PAGE_SHIFT; i += PAGE_SIZE) {
        ASSERT((p_pte = get_pte(g_bpt, i, 0)) != NULL);
        ASSERT(pte2va(*p_pte) == i);
    }

    ASSERT(g_bpt[0] == 0);

    struct page* p = alloc_page();
    ASSERT(add_page(g_bpt, p, 0x100, PTE_W | PTE_R | PTE_V) == 0);
    ASSERT(p->ref == 1);
    ASSERT(add_page(g_bpt, p, 0x100 + PAGE_SIZE, PTE_W | PTE_R | PTE_V) == 0);
    ASSERT(p->ref == 2);

    pte_t* t_pte1 = get_pte(g_bpt, 0x100, 0);
    pte_t* t_pte2 = get_pte(g_bpt, 0x100 + PAGE_SIZE, 0);

    ppn_t t_ppn1 = pte2ppn(*t_pte1);
    ppn_t t_ppn2 = pte2ppn(*t_pte2);

    u64 t_addr1 = pte2va(*t_pte1);
    u64 t_addr2 = pte2va(*t_pte2);

    const char *str = "ucore: Hello world!!";
    strcpy((void *)0x100, str);
    ASSERT(strcmp((void *)0x100, (void *)(0x100 + PAGE_SIZE)) == 0);

    *(char *)(page2va(p) + 0x100) = '\0';
    ASSERT(strlen((const char *)0x100) == 0);

    p_gpt1 = g_bpt;
    p_gpt2 = (pte_t*) page2va(pte2page(g_bpt[0]));
    free_page(p);
    free_page(pte2page(p_gpt1[0]));
    free_page(pte2page(p_gpt2[0]));
    g_bpt[0] = 0;

    ASSERT(num_free_pages == count_free_pages());

    putstr("test_page() succeed!\n\n");
}

// va2pa_mmu - Simulate RISC-V MMU address translation process with debug output
// This function walks through the 3-level page table (Sv39) to translate
// a virtual address to a physical address, exactly as the MMU hardware does.
//
// RISC-V Sv39 page table structure:
// - Level 1: GPT (Gigapage Table) - 512 entries, indexed by VPN[2] (bits 30:38)
// - Level 2: MPT (Megapage Table) - 512 entries, indexed by VPN[1] (bits 21:29)
// - Level 3: PT  (Page Table)     - 512 entries, indexed by VPN[0] (bits 12:20)
//
// @p_gpt: pointer to the Gigapage Table (root of page table)
// @va: virtual address to translate
// @return: physical address if translation succeeds, 0 if translation fails
u64 va2pa_mmu(pte_t* p_gpt, u64 va) {
    if (!p_gpt) {
        putstr("[MMU] Error: p_gpt is NULL\n");
        return 0;
    }

    putstr("\n=== RISC-V MMU Address Translation ===\n");
    putstr("Virtual Address: 0x"); putstr(u64toa(va, 16)); putch('\n');

    // Extract VPN and offset from virtual address
    u64 vpn2 = get_gptx(va);      // VPN[2] - bits 30:38
    u64 vpn1 = get_mptx(va);      // VPN[1] - bits 21:29
    u64 vpn0 = get_ptx(va);       // VPN[0] - bits 12:20
    u64 offset = get_offset(va);  // offset - bits 0:11

    putstr("\n[Step 1] Extract VPN and offset:\n");
    putstr("  VPN[2] = 0x"); putstr(u64toa(vpn2, 16)); putstr(" (bits 30:38)\n");
    putstr("  VPN[1] = 0x"); putstr(u64toa(vpn1, 16)); putstr(" (bits 21:29)\n");
    putstr("  VPN[0] = 0x"); putstr(u64toa(vpn0, 16)); putstr(" (bits 12:20)\n");
    putstr("  offset = 0x"); putstr(u64toa(offset, 16)); putstr(" (bits 0:11)\n");

    // Level 1: GPT (Gigapage Table)
    putstr("\n[Step 2] Level 1 - GPT (Gigapage Table):\n");
    putstr("  GPT base VA: 0x"); putstr(u64toa((u64) p_gpt, 16)); putch('\n');
    // GPT 的物理地址可以通过 va2page 获取
    struct page* gpt_page = va2page((u64) p_gpt);
    u64 gpt_pa = page2pa(gpt_page);
    putstr("  GPT base PA: 0x"); putstr(u64toa(gpt_pa, 16)); putch('\n');
    putstr("  Index (VPN[2]): "); putstr(u64toa(vpn2, 10)); putch('\n');

    if (vpn2 >= PT_SIZE) {
        putstr("  ERROR: Invalid VPN[2] index\n");
        return 0;
    }
    
    pte_t gpte = p_gpt[vpn2];
    putstr("  GPT["); putstr(u64toa(vpn2, 10)); putstr("] = 0x"); putstr(u64toa(gpte, 16)); putch('\n');

    if (!(gpte & PTE_V)) {
        putstr("  ERROR: GPT entry not valid (PTE_V = 0)\n");
        return 0;
    }

    // Extract PPN from GPT entry and get MPT base address
    ppn_t gpt_ppn = pte2ppn(gpte);
    u64 mpt_pa = gpt_ppn << PAGE_SHIFT;
    u64 mpt_va = pa2va(mpt_pa);
    pte_t* p_mpt = (pte_t*) mpt_va;
    
    putstr("  PPN from GPT: 0x"); putstr(u64toa(gpt_ppn, 16)); putch('\n');
    putstr("  MPT base PA: 0x"); putstr(u64toa(mpt_pa, 16)); putch('\n');
    putstr("  MPT base VA: 0x"); putstr(u64toa(mpt_va, 16)); putch('\n');

    // Level 2: MPT (Megapage Table)
    putstr("\n[Step 3] Level 2 - MPT (Megapage Table):\n");
    putstr("  Index (VPN[1]): "); putstr(u64toa(vpn1, 10)); putch('\n');

    if (vpn1 >= PT_SIZE) {
        putstr("  ERROR: Invalid VPN[1] index\n");
        return 0;
    }
    
    pte_t mpte = p_mpt[vpn1];
    putstr("  MPT["); putstr(u64toa(vpn1, 10)); putstr("] = 0x"); putstr(u64toa(mpte, 16)); putch('\n');

    if (!(mpte & PTE_V)) {
        putstr("  ERROR: MPT entry not valid (PTE_V = 0)\n");
        return 0;
    }

    // Extract PPN from MPT entry and get PT base address
    ppn_t mpt_ppn = pte2ppn(mpte);
    u64 pt_pa = mpt_ppn << PAGE_SHIFT;
    u64 pt_va = pa2va(pt_pa);
    pte_t* p_pt = (pte_t*) pt_va;
    
    putstr("  PPN from MPT: 0x"); putstr(u64toa(mpt_ppn, 16)); putch('\n');
    putstr("  PT base PA: 0x"); putstr(u64toa(pt_pa, 16)); putch('\n');
    putstr("  PT base VA: 0x"); putstr(u64toa(pt_va, 16)); putch('\n');

    // Level 3: PT (Page Table)
    putstr("\n[Step 4] Level 3 - PT (Page Table):\n");
    putstr("  Index (VPN[0]): "); putstr(u64toa(vpn0, 10)); putch('\n');

    if (vpn0 >= PT_SIZE) {
        putstr("  ERROR: Invalid VPN[0] index\n");
        return 0;
    }
    
    pte_t pte = p_pt[vpn0];
    putstr("  PT["); putstr(u64toa(vpn0, 10)); putstr("] = 0x"); putstr(u64toa(pte, 16)); putch('\n');

    if (!(pte & PTE_V)) {
        putstr("  ERROR: PT entry not valid (PTE_V = 0)\n");
        return 0;
    }

    // Extract PPN from final PTE
    ppn_t pt_ppn = pte2ppn(pte);
    
    putstr("  PPN from PT: 0x"); putstr(u64toa(pt_ppn, 16)); putch('\n');
    putstr("  Flags: ");
    if (pte & PTE_R) putstr("R ");
    if (pte & PTE_W) putstr("W ");
    if (pte & PTE_X) putstr("X ");
    if (pte & PTE_U) putstr("U ");
    if (pte & PTE_G) putstr("G ");
    if (pte & PTE_A) putstr("A ");
    if (pte & PTE_D) putstr("D ");
    putch('\n');

    // Construct physical address: PPN[2:0] | offset
    u64 ppn = pt_ppn;
    u64 pa = (ppn << PAGE_SHIFT) | offset;

    putstr("\n[Result] Physical Address:\n");
    putstr("  PA = (PPN << 12) | offset\n");
    putstr("  PA = (0x"); putstr(u64toa(ppn, 16)); putstr(" << 12) | 0x"); putstr(u64toa(offset, 16)); putstr("\n");
    putstr("  PA = 0x"); putstr(u64toa(pa, 16)); putstr("\n");
    putstr("========================================\n\n");

    return pa;
}
