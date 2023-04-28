#include "vmm.h"

#include "assert.h"
#include "list.h"
#include "pmm.h"
#include "defs.h"
#include "errno.h"
#include "memlayout.h"
#include "page.h"
#include "stdio.h"
#include "stdlib.h"
#include "swap.h"


struct vm_manager* g_check_vmm;

volatile uint32_t g_num_page_fault = 0;

static void check_vma_struct(void);
static void check_page_fault(void);

// init_vmm - alloc a vm_manager & initialize it
struct vm_manager* init_vmm() {
    struct vm_manager* vmm = malloc(sizeof(struct vm_manager));
    if (vmm) {
        init_list(&vmm->vma_list);
        vmm->vma_curr = NULL;
        vmm->p_gpte = NULL;
        vmm->vma_count = 0;

        if (g_swap_init_flag)
            swap_init_vmm(vmm);
        else 
            vmm->pra_list = NULL;
    }
    return vmm;
}

// del_vmm - free vmm and vmm internal fields
void del_vmm(struct vm_manager* vmm) {
    struct list* list = &vmm->vma_list;
    while ((list = list->next) != &vmm->vma_list) {
        del_list(list);
        free(LIST2VMA(list_link, list), sizeof(struct vm_area));
    }
    free(vmm, sizeof(struct vm_manager));
}

struct vm_area* init_vma(uintptr_t vm_start, uintptr_t vm_end, uint32_t vm_flags) {
    struct vm_area* vma = malloc(sizeof(struct vm_area));
    if (vma) {
        vma->vm_start = vm_start;
        vma->vm_end = vm_end;
        vma->vm_flags = vm_flags;
    }
    return vma;
}

struct vm_area *find_vma(struct vm_manager *vmm, uintptr_t addr) {
    struct vm_area* vma = NULL;
    if (vmm) {
        vma = vmm->vma_curr;
        if (!(vma && vma->vm_start <= addr && vma->vm_end > addr)) {
            bool found = 0;
            struct list* list = &(vmm->vma_list);
            while ((list = list->next) != &(vmm->vma_list)) {
                vma = LIST2VMA(list_link, list);
                if (vma->vm_start <= addr && vma->vm_end > addr) {
                    found = 1;
                    break;
                }
            }
            if (!found)
                vma = NULL;
        }
        if (vma)
            vmm->vma_curr = vma;
    }
    return vma;
}

static inline void check_vma_overlap(struct vm_area* vma_prev, struct vm_area* vma_next) {
    ASSERT(vma_prev->vm_start < vma_prev->vm_end);
    ASSERT(vma_prev->vm_end <= vma_next->vm_start);
    ASSERT(vma_next->vm_start < vma_next->vm_end);
}

// add_vma - insert vma in vmm's vma_list
void add_vma(struct vm_manager* vmm, struct vm_area* vma) {
    ASSERT(vma->vm_start < vma->vm_end);
    struct list* list = &vmm->vma_list;
    while ((list = list->next) != &vmm->vma_list &&
            ((LIST2VMA(list_link, list))->vm_start < vma->vm_start)) 
        continue;

    if (list->prev != &vmm->vma_list)
        check_vma_overlap(LIST2VMA(list_link, list->prev), vma);
    if (list != &vmm->vma_list)
        check_vma_overlap(vma, LIST2VMA(list_link, list));

    vma->vmm = vmm;
    add_list(list->prev, list, &vma->list_link);
    vmm->vma_count++;
}

void check_vmm() {
    size_t num_free_pages = count_free_pages();
    check_vma_struct();
    check_page_fault();

    // Sv39第二级页表多占了一个内存页
    ASSERT(num_free_pages - 1 == count_free_pages());

    putstr("check_vmm succeeded\n\n");
}

int32_t handle_page_fault(struct vm_manager* vmm, uintptr_t addr) {
    int32_t errno = EINVAL;
    // try to find a vma which include addr
    struct vm_area* vma = find_vma(vmm, addr);

    g_num_page_fault++;
    // If the addr is in the range of a vmm's vma
    if (!vma || vma->vm_start > addr) {
        putstr("invalid addr 0x"); putstr(uptrtoa(addr, 16));
        putstr(", and can not find it in vma\n");
        return errno;
    }

    uint32_t pte_flags = 0;
    if (vma->vm_flags & VMA_WRITE)
        pte_flags |= PTE_R | PTE_W;
    addr = rounddown(addr, PAGE_SIZE);
    errno = ENOMEM;

    pte_t* p_pte = get_pte(vmm->p_gpte, addr, 1);
    if (!(*p_pte)) {
        if (!alloc_page_gpte(vmm->p_gpte, addr, pte_flags)) {
            WARN("alloc_page_gpte fail");
            return errno;
        }
    } else {
        if (g_swap_init_flag) {
            struct page* page = NULL;
            swap_in(vmm, addr, &page);
            add_page(vmm->p_gpte, page, addr, pte_flags);
            swap_set_swappable(vmm, page);
            page->pra_addr = addr;
        } else {
            putstr("no swap_init but p_pte is 0x"); putstr(uptrtoa(*p_pte, 16));
            putstr(" failed\n");
            return errno;
        }
    }
    return 0;
}

static void check_vma_struct(void) {
    size_t num_free_pages = count_free_pages();

    struct vm_manager* vmm = init_vmm();
    ASSERT(vmm != NULL);

    int32_t step1 = 10;
    int32_t step2 = step1 * 10;

    for (int32_t i = step1; i >= 1; i--) {
        struct vm_area* vma = init_vma(i * 5, i * 5 + 2, 0);
        ASSERT(vma != NULL);
        add_vma(vmm, vma);
    }

    for (int32_t i = step1 + 1; i <= step2; i++) {
        struct vm_area* vma = init_vma(i * 5, i * 5 + 2, 0);
        ASSERT(vma != NULL);
        add_vma(vmm, vma);
    }

    struct list* list = vmm->vma_list.next;

    for (int32_t i = 1; i <= step2; i++) {
        ASSERT(list != &vmm->vma_list);
        struct vm_area* vma = LIST2VMA(list_link, list);
        ASSERT(vma->vm_start == i * 5 && vma->vm_end == i * 5 + 2);
        list = list->next;
    }

    for (int32_t i = 5; i <= 5 * step2; i += 5) {
        struct vm_area *vma1 = find_vma(vmm, i);
        ASSERT(vma1 != NULL);
        struct vm_area *vma2 = find_vma(vmm, i+1);
        ASSERT(vma2 != NULL);
        struct vm_area *vma3 = find_vma(vmm, i+2);
        ASSERT(vma3 == NULL);
        struct vm_area *vma4 = find_vma(vmm, i+3);
        ASSERT(vma4 == NULL);
        struct vm_area *vma5 = find_vma(vmm, i+4);
        ASSERT(vma5 == NULL);

        ASSERT(vma1->vm_start == i  && vma1->vm_end == i  + 2);
        ASSERT(vma2->vm_start == i  && vma2->vm_end == i  + 2);
    }

    for (int32_t i = 4; i >= 0; i--) {
        struct vm_area *vma_below_5= find_vma(vmm,i);
        if (vma_below_5) {
           putstr("vma_below_5: i 0x"); putstr(i32toa(i, 16));
           putstr(", start 0x"); putstr(uptrtoa(vma_below_5->vm_start, 16));
           putstr(", end 0x"); putstr(uptrtoa(vma_below_5->vm_end, 16));
           putch('\n');
        }
        ASSERT(vma_below_5 == NULL);
    }

    del_vmm(vmm);

    ASSERT(num_free_pages == count_free_pages());

    putstr("check_vma_struct() succeeded!\n");
}


// check_page_fault - check correctness of page_fault handler
static void check_page_fault(void) {
	// char *name = "check_page_fault";
    size_t num_free_pages = count_free_pages();

    g_check_vmm = init_vmm();

    ASSERT(g_check_vmm != NULL);
    struct vm_manager *vmm = g_check_vmm;
    pte_t* p_gpte = vmm->p_gpte = g_satp;
    ASSERT(p_gpte[0] == 0);

    struct vm_area *vma = init_vma(0, MP_SIZE, VMA_WRITE);

    ASSERT(vma != NULL);

    add_vma(vmm, vma);

    uintptr_t addr = 0x100;
    ASSERT(find_vma(vmm, addr) == vma);

    int32_t sum = 0;
    for (int32_t i = 0; i < 100; i ++) {
        *(char *)(addr + i) = i;
        sum += i;
    }
    for (int32_t i = 0; i < 100; i ++) {
        sum -= *(char *)(addr + i);
    }
    ASSERT(sum == 0);

    del_page(p_gpte, rounddown(addr, PAGE_SIZE));

    free_page(pte2page(p_gpte[0]));

    p_gpte[0] = 0;

    vmm->p_gpte = NULL;
    del_vmm(vmm);

    g_check_vmm = NULL;

    // Sv39第二级页表多占了一个内存页
    ASSERT(num_free_pages - 1 == count_free_pages());

    putstr("check_page_fault() succeeded!\n");
}

