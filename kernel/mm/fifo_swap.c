#include "fifo_swap.h"

#include "assert.h"
#include "list.h"
#include "page.h"
#include "swap.h"
#include "vmm.h"


static struct list g_pra_list;

// fifo_init_vmm - init g_pra_list and let vmm->pra_list point to the addr of g_pra_list.
//      Now, from the memory control struct vmm, we can access FIFO PRA
static void fifo_init_vmm(struct vm_manager* vmm) {
    init_list(&g_pra_list);
    vmm->pra_list = &g_pra_list;
}

static void fifo_set_unswappable(struct vm_manager* vmm, struct page* page) { }

static void fifo_set_swappable(struct vm_manager* vmm, struct page* page) {
    add_list(vmm->pra_list, vmm->pra_list->next, &page->pra_list_link);
}

static void fifo_swap_out_page(struct vm_manager* vmm, struct page** p_page) {
    ASSERT(!empty_list(vmm->pra_list));
    struct list* list = vmm->pra_list->prev;
    del_list(list);
    *p_page = LIST2PAGE(pra_list_link, list);
}

static void check_fifo() {
    // 0x5000 -> 0x4000 -> 0x3000 -> 0x2000 -> 0x1000
    //    e   ->    d   ->    c   ->    b   ->    a

    putstr("write Virt Page c in check_fifo\n");
    *(byte*) 0x3000 = 0x0c;
    ASSERT(g_num_page_fault == 5);
    putstr("write Virt Page a in check_fifo\n");
    *(byte*) 0x1000 = 0x0a;
    ASSERT(g_num_page_fault == 5);
    putstr("write Virt Page d in check_fifo\n");
    *(byte*) 0x4000 = 0x0d;
    ASSERT(g_num_page_fault == 5);
    putstr("write Virt Page b in check_fifo\n");
    *(byte*) 0x2000 = 0x0b;
    ASSERT(g_num_page_fault == 5);
    putstr("write Virt Page e in check_fifo\n");
    *(byte*) 0x5000 = 0x0e;
    ASSERT(g_num_page_fault == 5);

    putstr("\nwrite Virt Page f in check_fifo (swap out a)\n");
    *(byte*) 0x6000 = 0x0f;
    ASSERT(g_num_page_fault == 6);
    putstr("\nwrite Virt Page b in check_fifo\n");
    *(byte*) 0x2000 = 0x0b;
    ASSERT(g_num_page_fault == 6);

    putstr("\nwrite Virt Page a in check_fifo (swap out b)\n");
    *(byte*) 0x1000 = 0x0a;
    ASSERT(g_num_page_fault == 7);

    putstr("\nwrite Virt Page b in check_fifo (swap out c)\n");
    *(byte*) 0x2000 = 0x0b;
    ASSERT(g_num_page_fault == 8);

    putstr("\nwrite Virt Page c in check_fifo (swap out d)\n");
    *(byte*) 0x3000 = 0x0c;
    ASSERT(g_num_page_fault == 9);

    putstr("\nwrite Virt Page d in check_fifo (swap out e)\n");
    *(byte*) 0x4000 = 0x0d;
    ASSERT(g_num_page_fault == 10);

    putstr("\nwrite Virt Page e in check_fifo (swap out f)\n");
    *(byte*) 0x5000 = 0x0e;
    ASSERT(g_num_page_fault == 11);

    putstr("\nwrite Virt Page f in check_fifo (swap out a)\n");
    *(byte*) 0x6000 = 0x0f;
    ASSERT(g_num_page_fault == 12);

    putstr("\nwrite Virt Page a in check_fifo (swap out b)\n");
    ASSERT(*(byte*) 0x1000 == 0x0a);
    *(byte*) 0x1000 = 0x0a;
    ASSERT(g_num_page_fault == 13);
}

const struct swap_manager g_fifo_swap_manager = {
    .name = "fifo_swap_manager",
    .init_vmm = fifo_init_vmm,
    .set_swappable = fifo_set_swappable,
    .set_unswappable = fifo_set_unswappable,
    .swap_out_page = fifo_swap_out_page,
    .check = check_fifo
};
