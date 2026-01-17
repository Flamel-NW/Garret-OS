#ifndef __KERNEL_MM_SWAP_H__
#define __KERNEL_MM_SWAP_H__

#include "assert.h"
#include "defs.h"
#include "pmm.h"
#include "vmm.h"
#include "ide.h"

// Sv39 page table entry:
// +----26---+----9---+----9---+---2----+-------8-------+
// |  PPN[2] | PPN[1] | PPN[0] |Reserved|D|A|G|U|X|W|R|V|
// +---------+----+---+--------+--------+---------------+
// swap_entry_t:
// +-----------------24-----------------+-----7-----+-1-+
// |               offset               | reserved  | 0 |
// +------------------------------------+-----------+---+
// 
// 当一个PTE用来描述一般意义上的物理页时, 显然它应该维护各种权限和映射关系, 以及应该有PTE_V标记; 
// 但当它用来描述一个被置换出去的物理页时, 它被用来维护该物理页与 swap 磁盘上扇区的映射关系, 
// 并且该 PTE 不应该由 MMU 将它解释成物理页映射(即没有 PTE_V 标记), 
// 与此同时对应的权限则交由 vmm 来维护, 当对位于该页的内存地址进行访问的时候, 
// 必然导致 page fault, 然后ucore能够根据 PTE 描述的 swap 项将相应的物理页重新建立起来, 
// 并根据虚存所描述的权限重新设置好 PTE 使得内存访问能够继续正常进行。

// 如果一个页(4KB/页)被置换到了硬盘某8个扇区(0.5KB/扇区), 
// 该PTE的最低位--present位应该为0 (即 PTE_P 标记为空, 表示虚实地址映射关系不存在), 
// 接下来的7位暂时保留, 可以用作各种扩展; 而包括原来高20位页帧号的高24位数据, 
// 恰好可以用来表示此页在硬盘上的起始扇区的位置(其从第几个扇区开始)
// 为了在页表项中区别 0 和 swap 分区的映射, 将 swap 分区的一个 page 空出来不用, 
// 也就是说一个高24位不为0, 而最低位为0的PTE表示了一个放在硬盘上的页的起始扇区号

typedef u64 swap_entry_t;

#define SWAP_SHIFT          8
#define PAGE_SECT           (PAGE_SIZE / SECTION_SIZE)
#define SWAP_MAX_OFFSET     (IDE_SECTIONS / PAGE_SECT)

static inline swap_entry_t addr2swap(u64 addr) {
    return (addr >> PAGE_SHIFT) << SWAP_SHIFT;
}

static inline u64 swap_offset(swap_entry_t entry) {
    u64 offset = entry >> SWAP_SHIFT;
    ASSERT(offset > 0 && offset < SWAP_MAX_OFFSET);
    return offset;
}

static inline void read_swap_ide(swap_entry_t entry, struct page* page) {
    read_ide_sects((void*) page2va(page), swap_offset(entry) * PAGE_SECT, PAGE_SECT);
}

static inline void write_swap_ide(swap_entry_t entry, struct page* page) {
    write_ide_sects((void*) page2va(page), swap_offset(entry) * PAGE_SECT, PAGE_SECT);
}


extern volatile bool g_swap_init_flag;

struct swap_manager {
    const char* name;

    // Initialization the private data inside vmm
    void (*init_vmm) (struct vm_manager* vmm);

    // Called when map a swappable page into the vmm
    void (*set_swappable) (struct vm_manager* vmm, struct page* page);
    // When a page is marked as shared, this routine is called to delete the page from the swap manager
    void (*set_unswappable) (struct vm_manager* vmm, struct page* page);

    // Try to swap out a page
    void (*swap_out_page) (struct vm_manager* vmm, struct page** p_page);

    // test the page replacement algorithm
    void (*test) ();
};

void swap_init_vmm(struct vm_manager* vmm);
void swap_set_swappable(struct vm_manager* vmm, struct page* page);
void swap_set_unswappable(struct vm_manager* vmm, struct page* page);

void init_swap();

void swap_out(struct vm_manager* vmm, u64 num_pages);
void swap_in(struct vm_manager* vmm, u64 addr, struct page** p_page);

#endif // __KERNEL_MM_SWAP_H__
