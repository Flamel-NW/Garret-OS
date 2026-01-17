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


struct vm_manager* g_test_vmm;

volatile u32 g_num_page_fault = 0;

static void test_vma_struct();
static void test_page_fault();

// init_vmm - alloc a vm_manager & initialize it
struct vm_manager* init_vmm() {
    struct vm_manager* vmm = malloc(sizeof(struct vm_manager));
    if (vmm) {
        init_list(&vmm->vma_list);
        vmm->vma_curr = NULL;
        vmm->p_gpt = NULL;
        vmm->vma_count = 0;

        if (g_swap_init_flag)
            swap_init_vmm(vmm);
        else 
            vmm->pra_list = NULL;

        vmm->use_count = 0;
        init_lock(&(vmm->lock));
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

i32 vm_map(struct vm_manager* vmm, u64 addr, u64 len, u32 vm_flags, struct vm_area** vma_store) {
    u64 start = rounddown(addr, PAGE_SIZE);
    u64 end = roundup(addr + len, PAGE_SIZE);

    if (!user_access(start, end)) 
        return -E_INVAL;

    ASSERT(vmm != NULL);

    struct vm_area* vma;
    if ((vma = find_vma(vmm, start)) && end > vma->vm_start)
        return -E_INVAL;

    if (!(vma = init_vma(start, end, vm_flags)))
        return -E_NO_DEV;

    add_vma(vmm, vma);
    if (vma_store)
        *vma_store = vma;

    return 0;
}

i32 dup_vmm(struct vm_manager* from, struct vm_manager* to) {
    ASSERT(to != NULL && from != NULL);
    struct list* list = &from->vma_list;
    while ((list = list->prev) != &from->vma_list) {
        struct vm_area* vma = LIST2VMA(list_link, list);
        struct vm_area* new_vma = init_vma(vma->vm_start, vma->vm_end, vma->vm_flags);
        if (!new_vma)
            return -E_NO_MEM;

        add_vma(to, new_vma);
        if (copy_range(from->p_gpt, to->p_gpt, vma->vm_start, vma->vm_end))
            return -E_NO_MEM;
    }
    return 0;
}

// cleanup all vm_areas in the vmm
void del_vmas(struct vm_manager* vmm) {
    ASSERT(vmm != NULL && vmm->use_count == 0);
    pte_t* p_gpt = vmm->p_gpt;
    struct list* list = &vmm->vma_list;
    while ((list = list->next) != &vmm->vma_list) {
        struct vm_area* vma = LIST2VMA(list_link, list);
        del_range(p_gpt, vma->vm_start, vma->vm_end);
    }
    while ((list = list->next) != &vmm->vma_list) {
        struct vm_area* vma = LIST2VMA(list_link, list);
        free_range(p_gpt, vma->vm_start, vma->vm_end);
    }
}

struct vm_area* init_vma(u64 vm_start, u64 vm_end, u32 vm_flags) {
    struct vm_area* vma = malloc(sizeof(struct vm_area));
    if (vma) {
        vma->vm_start = vm_start;
        vma->vm_end = vm_end;
        vma->vm_flags = vm_flags;
    }
    return vma;
}

struct vm_area *find_vma(struct vm_manager *vmm, u64 addr) {
    struct vm_area* vma = NULL;
    if (vmm) {
        vma = vmm->vma_curr;
        if (!(vma && vma->vm_start <= addr && vma->vm_end > addr)) {
            bool is_found = false;
            struct list* list = &(vmm->vma_list);
            while ((list = list->next) != &(vmm->vma_list)) {
                vma = LIST2VMA(list_link, list);
                if (vma->vm_start <= addr && vma->vm_end > addr) {
                    is_found = true;
                    break;
                }
            }
            if (!is_found)
                vma = NULL;
        }
        if (vma)
            vmm->vma_curr = vma;
    }
    return vma;
}

static inline void test_vma_overlap(struct vm_area* vma_prev, struct vm_area* vma_next) {
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
        test_vma_overlap(LIST2VMA(list_link, list->prev), vma);
    if (list != &vmm->vma_list)
        test_vma_overlap(vma, LIST2VMA(list_link, list));

    vma->vmm = vmm;
    add_list(list->prev, list, &vma->list_link);
    vmm->vma_count++;
}

void test_vmm() {
    u64 num_free_pages = count_free_pages();
    test_vma_struct();
    test_page_fault();

    // Sv39第二级页表多占了一个内存页
    ASSERT(num_free_pages - 1 == count_free_pages());

    putstr("test_vmm succeed!\n\n");
}

i32 handle_page_fault(struct vm_manager* vmm, u64 addr) {
    i32 errno = -E_INVAL;
    // try to find a vma which include addr
    struct vm_area* vma = find_vma(vmm, addr);

    g_num_page_fault++;
    // If the addr is in the range of a vmm's vma
    if (!vma || vma->vm_start > addr) {
        putstr("invalid addr 0x"); putstr(u64toa(addr, 16));
        putstr(", and can not find it in vma\n");
        return errno;
    }

    u32 pte_flags = 0;
    if (vma->vm_flags & VMA_WRITE)
        pte_flags |= PTE_R | PTE_W;
    addr = rounddown(addr, PAGE_SIZE);
    errno = -E_NO_MEM;

    pte_t* p_pte = get_pte(vmm->p_gpt, addr, 1);
    if (!(*p_pte)) {
        if (!alloc_page_gpte(vmm->p_gpt, addr, pte_flags)) {
            WARN("alloc_page_gpte fail");
            return errno;
        }
    } else {
        if (g_swap_init_flag) {
            struct page* page = NULL;
            swap_in(vmm, addr, &page);
            add_page(vmm->p_gpt, page, addr, pte_flags);
            swap_set_swappable(vmm, page);
            page->pra_addr = addr;
        } else {
            putstr("no swap_init but p_pte is 0x"); putstr(u64toa(*p_pte, 16));
            putstr(" failed\n");
            return errno;
        }
    }
    return 0;
}

static void test_vma_struct() {
    u64 num_free_pages = count_free_pages();

    struct vm_manager* vmm = init_vmm();
    ASSERT(vmm != NULL);

    i32 step1 = 10;
    i32 step2 = step1 * 10;

    for (i32 i = step1; i >= 1; i--) {
        struct vm_area* vma = init_vma(i * 5, i * 5 + 2, 0);
        ASSERT(vma != NULL);
        add_vma(vmm, vma);
    }

    for (i32 i = step1 + 1; i <= step2; i++) {
        struct vm_area* vma = init_vma(i * 5, i * 5 + 2, 0);
        ASSERT(vma != NULL);
        add_vma(vmm, vma);
    }

    struct list* list = vmm->vma_list.next;

    for (i32 i = 1; i <= step2; i++) {
        ASSERT(list != &vmm->vma_list);
        struct vm_area* vma = LIST2VMA(list_link, list);
        ASSERT(vma->vm_start == i * 5 && vma->vm_end == i * 5 + 2);
        list = list->next;
    }

    for (i32 i = 5; i <= 5 * step2; i += 5) {
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

    for (i32 i = 4; i >= 0; i--) {
        struct vm_area *vma_below_5= find_vma(vmm,i);
        if (vma_below_5) {
           putstr("vma_below_5: i 0x"); putstr(i32toa(i, 16));
           putstr(", start 0x"); putstr(u64toa(vma_below_5->vm_start, 16));
           putstr(", end 0x"); putstr(u64toa(vma_below_5->vm_end, 16));
           putch('\n');
        }
        ASSERT(vma_below_5 == NULL);
    }

    del_vmm(vmm);

    ASSERT(num_free_pages == count_free_pages());

    putstr("test_vma_struct() succeed!\n\n");
}


// test_page_fault - test correctness of page_fault handler
static void test_page_fault() {
	// char *name = "test_page_fault";
    u64 num_free_pages = count_free_pages();

    g_test_vmm = init_vmm();

    ASSERT(g_test_vmm != NULL);
    struct vm_manager *vmm = g_test_vmm;
    pte_t* p_gpt = vmm->p_gpt = g_bpt;
    ASSERT(p_gpt[0] == 0);

    struct vm_area *vma = init_vma(0, MP_SIZE, VMA_WRITE);

    ASSERT(vma != NULL);

    add_vma(vmm, vma);

    u64 addr = 0x100;
    ASSERT(find_vma(vmm, addr) == vma);

    i32 sum = 0;
    for (i32 i = 0; i < 100; i ++) {
        *(char *)(addr + i) = i;
        sum += i;
    }
    for (i32 i = 0; i < 100; i ++) {
        sum -= *(char *)(addr + i);
    }
    ASSERT(sum == 0);

    del_page(p_gpt, rounddown(addr, PAGE_SIZE));

    free_page(pte2page(p_gpt[0]));

    p_gpt[0] = 0;

    vmm->p_gpt = NULL;
    del_vmm(vmm);

    g_test_vmm = NULL;

    // Sv39第二级页表多占了一个内存页
    ASSERT(num_free_pages - 1 == count_free_pages());

    putstr("\ntest_page_fault() succeed!\n\n");
}

bool check_vmm(struct vm_manager* vmm, u64 addr, u64 len, bool write) {
    if (vmm) {
        if (!user_access(addr, addr + len))
            return false;
        struct vm_area* vma;
        u64 begin = addr;
        u64 end = addr + len;
        while (begin < end) {
            if (!(vma = find_vma(vmm, begin)) || 
                    begin < vma->vm_start ||
                    !(vma->vm_flags & (write ? VMA_WRITE : VMA_READ)) ||
                    (write && (vma->vm_flags & VMA_STACK) &&
                    begin < vma->vm_start + PAGE_SIZE))
                return false;
            begin = vma->vm_end;
        }
        return true;
    } else {
        return kernel_access(addr, addr + len);
    }
}
