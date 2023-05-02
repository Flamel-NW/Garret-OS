#ifndef __KERNEL_MM_VMM_H__
#define __KERNEL_MM_VMM_H__

#include "defs.h"
#include "list.h"
#include "page.h"
#include "errno.h"

static inline void flush_tlb() {
    __asm__ volatile ( "sfence.vma" );
}

struct vm_area;

struct vm_manager;

// the virtual continuous memory area(vma), [vm_start, vm_end),
// addr belong to vma means vma.vm_start <= addr < vma_end
struct vm_area {
    struct list list_link;  // linear list link which sorted by start addr of vma
    struct vm_manager* vmm; // the set of vma using the same GPT
    uintptr_t vm_start;     // start addr of vma
    uintptr_t vm_end;       // end addr of vma, not include the vm_end itself
    uint32_t vm_flags;      // flags of vma
};

#define VMA_READ    0
#define VMA_WRITE   1
#define VMA_EXEC    2

#define LIST2VMA(member, list)  \
    TO_STRUCT(struct vm_area, member, (list))


// the control struct of the set of vma in the same gigapage
struct vm_manager {
    struct list vma_list;       // linear list link which sorted by start addr of vma
    struct vm_area* vma_curr;   // current accessed vma, used for speed purpose
    pte_t* p_gpte;              // the GPTE of these vma
    uint32_t vma_count;         // the count of these vma
    struct list* pra_list;      // the private data for swap manager
};

extern struct vm_manager* g_check_vmm;

extern volatile uint32_t g_num_page_fault;

void check_vmm();

// init_vmm - alloc a vm_manager & initialize it
struct vm_manager* init_vmm();

struct vm_area* init_vma(uintptr_t vm_start, uintptr_t vm_end, uint32_t vm_flags);

// add_vma - insert vma in vmm's vma_list
void add_vma(struct vm_manager* vmm, struct vm_area* vma);

struct vm_area *find_vma(struct vm_manager *vmm, uintptr_t addr);
int32_t handle_page_fault(struct vm_manager* vmm, uintptr_t addr);

#endif
