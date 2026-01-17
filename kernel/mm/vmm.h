#ifndef __KERNEL_MM_VMM_H__
#define __KERNEL_MM_VMM_H__

#include "defs.h"
#include "list.h"
#include "page.h"
#include "errno.h"
#include "sync.h"


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
    u64 vm_start;     // start addr of vma
    u64 vm_end;       // end addr of vma, not include the vm_end itself
    u32 vm_flags;      // flags of vma
};

#define VMA_READ    0x00000001
#define VMA_WRITE   0x00000002
#define VMA_EXEC    0x00000004
#define VMA_STACK   0x00000008

#define LIST2VMA(member, list)  \
    TO_STRUCT(struct vm_area, member, (list))


// the control struct of the set of vma in the same gigapage
struct vm_manager {
    struct list vma_list;       // linear list link which sorted by start addr of vma
    struct vm_area* vma_curr;   // current accessed vma, used for speed purpose
    pte_t* p_gpt;               // the GPT of these vma
    u32 vma_count;              // the count of these vma
    struct list* pra_list;      // the private data for swap manager
    u32 use_count;              // the number off process which shared the vmm
    lock_t lock;                // mutex for using dum_mmap fun to duplicat the vmm
};

extern struct vm_manager* g_test_vmm;

extern volatile u32 g_num_page_fault;

void test_vmm();

// init_vmm - alloc a vm_manager & initialize it
struct vm_manager* init_vmm();

// del_vmm - free vmm and vmm internal fields
void del_vmm(struct vm_manager* vmm);

struct vm_area* init_vma(u64 vm_start, u64 vm_end, u32 vm_flags);

// add_vma - insert vma in vmm's vma_list
void add_vma(struct vm_manager* vmm, struct vm_area* vma);

struct vm_area *find_vma(struct vm_manager *vmm, u64 addr);
i32 handle_page_fault(struct vm_manager* vmm, u64 addr);

bool check_vmm(struct vm_manager* vmm, u64 addr, u64 len, bool write);

i32 dup_vmm(struct vm_manager* from, struct vm_manager* to);

void del_vmas(struct vm_manager* vmm);

i32 vm_map(struct vm_manager* vmm, u64 addr, u64 len, u32 vm_flags, struct vm_area** vma_store);

#endif // __KERNEL_MM_VMM_H__
