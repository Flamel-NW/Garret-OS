#include "swap.h"

#include "assert.h"
#include "list.h"
#include "fifo_swap.h"
#include "defs.h"
#include "memlayout.h"
#include "page.h"
#include "pmm.h"
#include "stdio.h"
#include "stdlib.h"
#include "vmm.h"

volatile bool g_swap_init_flag;
static const struct swap_manager* g_swap_manager;

static void test_swap();

void swap_init_vmm(struct vm_manager* vmm) {
    g_swap_manager->init_vmm(vmm);
}

void swap_set_swappable(struct vm_manager* vmm, struct page* page) {
    g_swap_manager->set_swappable(vmm, page);
}

void swap_set_unswappable(struct vm_manager* vmm, struct page* page) {
    g_swap_manager->set_unswappable(vmm, page);
}

void init_swap() {
    g_swap_manager = &g_fifo_swap_manager;
    putstr("swap manager: "); putstr(g_swap_manager->name); putstr("\n\n");
    g_swap_init_flag = true;
    test_swap();
}

void swap_out(struct vm_manager* vmm, u64 num_pages) {
    while (num_pages--) {
        struct page* page;
        g_swap_manager->swap_out_page(vmm, &page);
    
        u64 addr = page->pra_addr;
        pte_t* p_pte = get_pte(vmm->p_gpt, addr, 0);
        ASSERT(*p_pte & PTE_V);

        write_swap_ide(addr2swap(page->pra_addr), page);
        putstr("swap out. store page in addr 0x"); putstr(u64toa(addr, 16));
        putstr(" to disk swap entry ");
        putstr(u64toa(addr2swap(page->pra_addr), 16));
        putch('\n');

        *p_pte = addr2swap(page->pra_addr);
        free_page(page);

        flush_tlb();
    }
}

void swap_in(struct vm_manager* vmm, u64 addr, struct page** p_page) {
    struct page* page = alloc_page();
    ASSERT(page);

    pte_t* p_pte = get_pte(vmm->p_gpt, addr, 0);
    ASSERT(!(*p_pte & PTE_V));
    putstr("swap in. load page in addr 0x"); putstr(u64toa(addr, 16));
    putstr(" from disk swap entry "); 
    putstr(u64toa(swap_offset(*p_pte), 16));
    putch('\n');

    read_swap_ide((*p_pte), page);

    *p_page = page;
    flush_tlb();
}


#define NUM_CHECK_PAGES 6
#define CHECK_VA_BEGIN  0x1000
#define CHECK_VA_END    (CHECK_VA_BEGIN + (NUM_CHECK_PAGES << PAGE_SHIFT))

#define MAX_SEQ         10

static inline void test_content_set()
{
    *(u8*) 0x1000 = 0x0a;
    ASSERT(g_num_page_fault == 1);
    *(u8*) 0x1010 = 0x0a;
    ASSERT(g_num_page_fault == 1);
    *(u8*) 0x2000 = 0x0b;
    ASSERT(g_num_page_fault == 2);
    *(u8*) 0x2010 = 0x0b;
    ASSERT(g_num_page_fault == 2);
    *(u8*) 0x3000 = 0x0c;
    ASSERT(g_num_page_fault == 3);
    *(u8*) 0x3010 = 0x0c;
    ASSERT(g_num_page_fault == 3);
    *(u8*) 0x4000 = 0x0d;
    ASSERT(g_num_page_fault == 4);
    *(u8*) 0x4010 = 0x0d;
    ASSERT(g_num_page_fault == 4);
    *(u8*) 0x5000 = 0x0e;
    ASSERT(g_num_page_fault == 5);
    *(u8*) 0x5010 = 0x0e;
    ASSERT(g_num_page_fault == 5);
}

static void test_swap() {
    u32 count = 0;
    u32 total = 0;
    struct list* list = &get_free_area()->free_list;
    while ((list = list->next) != &get_free_area()->free_list) {
        struct page* p = LIST2PAGE(free_list_link, list);
        ASSERT(p->flags & PAGE_PROPERTY);
        count++;
        total += p->property;
    }
    ASSERT(total == count_free_pages());
    
    putstr("begin test swap: count "); putstr(u32toa(count, 10));
    putstr(", total "); putstr(u32toa(total, 10)); putstr("\n\n");

    // now we set the physical pages environment
    struct vm_manager* vmm = init_vmm();
    ASSERT(vmm != NULL);
    ASSERT(g_test_vmm == NULL);
    g_test_vmm = vmm;

    vmm->p_gpt = g_bpt;
    ASSERT(vmm->p_gpt[0] == 0);

    struct vm_area* vma = init_vma(CHECK_VA_BEGIN, CHECK_VA_END, VMA_WRITE | VMA_READ);
    ASSERT(vma != NULL);

    add_vma(vmm, vma);

    putstr("setup Page Table for va 0x1000 ~ 0x6000, so alloc a page\n");
    pte_t* t_p_pte = get_pte(vmm->p_gpt, CHECK_VA_BEGIN, 1);
    ASSERT(t_p_pte != NULL);
    putstr("setup Page Table va 0x1000 ~ 0x6000 OVER!\n\n");

    struct page* test_pages[NUM_CHECK_PAGES];
    for (u32 i = 0; i < NUM_CHECK_PAGES - 1; i++) {
        test_pages[i] = alloc_page();
        ASSERT(test_pages[i] != NULL);
        ASSERT((test_pages[i]->flags & PAGE_PROPERTY) == 0);
    }
    struct free_area temp_free_area = *get_free_area();

    init_list(&get_free_area()->free_list);
    get_free_area()->num_free = 0;

    for (u32 i = 0; i < NUM_CHECK_PAGES - 1; i++) 
        free_page(test_pages[i]);

    ASSERT(get_free_area()->num_free == NUM_CHECK_PAGES - 1);

    putstr("set up init env for test begin: \n\n");

    g_num_page_fault = 0;
    test_content_set();
    ASSERT(get_free_area()->num_free == 0);

    u32 swap_in_seq[MAX_SEQ];
    u32 swap_out_seq[MAX_SEQ];

    for (u32 i = 0; i < MAX_SEQ; i++)
        swap_in_seq[i] = swap_out_seq[i] = -1;

    pte_t* test_p_ptes[NUM_CHECK_PAGES];
    for (u32 i = 0; i < NUM_CHECK_PAGES - 1; i++) {
        test_p_ptes[i] = get_pte(vmm->p_gpt, (i + 1) << PAGE_SHIFT, 0);
        ASSERT(test_p_ptes[i] != NULL);
        ASSERT(pte2page(*test_p_ptes[i]) == test_pages[i]);
        ASSERT((*test_p_ptes[i] & PTE_V) != 0);
    }

    putstr("\nset up init env for test over!\n\n");

    // now access the virt pages to test page relpacement algorithm 
    g_swap_manager->test();
    
    //restore kernel mem env
    for (u32 i = 0; i < NUM_CHECK_PAGES - 1; i++) 
        free_page(test_pages[i]);

    free_page(pte2page(vmm->p_gpt[0]));
    vmm->p_gpt[0] = 0;
    vmm->p_gpt = NULL;
    del_vmm(vmm);
    g_test_vmm = NULL;

    *get_free_area() = temp_free_area;
    
    list = &get_free_area()->free_list;
    while ((list = list->next) != &get_free_area()->free_list) {
        struct page* p = LIST2PAGE(free_list_link, list);
        count--;
        total -= p->property;
    }
    putstr("end test swap: count "); putstr(u32toa(count, 10));
    putstr(", total "); putstr(u32toa(total, 10));
    putstr("\n\n");

    g_bpt[0] = 0;
    
    putstr("test_swap() succeed!\n\n"); 
}
