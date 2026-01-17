#ifndef __KERNEL_MM_PAGE_H__
#define __KERNEL_MM_PAGE_H__

#include "memlayout.h"
#include "defs.h"
#include "list.h"

// Sv39 - V II: P84
// A linear address 'addr' has a four-part structure as follows:
//
// +--------9-------+-------9--------+-------9--------+---------12----------+
// | Gigapage Table | Megapage Table |   Page Table   | Offset within Page  |
// +----------------+----------------+----------------+---------------------+
// |-- GPTX(addr) --|-- MPTX(addr) --|--- PTX(addr) --|-- PAGE_OFF(addr) ---|
// |-------------------VPN(addr)----------------------|

// RISC-V uses 39-bit virtual address to access 56-bit physical address!
// Sv39 virtual address:
// +----9----+----9---+----9---+---12--+
// |  VPN[2] | VPN[1] | VPN[0] | PGOFF |
// +---------+----+---+--------+-------+
//
// Sv39 physical address:
// +----26---+----9---+----9---+---12--+
// |  PPN[2] | PPN[1] | PPN[0] | PGOFF |
// +---------+----+---+--------+-------+
//
// Sv39 page table entry:
// +----26---+----9---+----9---+---2----+-------8-------+
// |  PPN[2] | PPN[1] | PPN[0] |Reserved|D|A|G|U|X|W|R|V|
// +---------+----+---+--------+--------+---------------+

typedef u64 ppn_t;        // physical page number
typedef u64 pte_t;        // page table entry

#define PT_SHIFT    9 
#define PT_SIZE     (1 << PT_SHIFT)

#define MP_SIZE     (PAGE_SIZE << PT_SHIFT)
#define GP_SIZE     (MP_SIZE << PT_SHIFT)

#define PTX_SHIFT   12
#define MPTX_SHIFT  (PTX_SHIFT + PT_SHIFT)
#define GPTX_SHIFT  (MPTX_SHIFT + PT_SHIFT)


static inline u64 get_ptx(u64 addr) {
    return (addr >> PTX_SHIFT) & 0x1FF;
}

static inline u64 get_mptx(u64 addr) {
    return (addr >> MPTX_SHIFT) & 0x1FF;
}

static inline u64 get_gptx(u64 addr) {
    return (addr >> GPTX_SHIFT) & 0x1FF;
}

static inline u64 get_offset(u64 addr) {
    return addr & 0xFFF;
}

static inline u64 ptx2addr(u64 gptx, u64 mptx, 
                                 u64 ptx, u64 offset) {
    return (gptx << GPTX_SHIFT) | (mptx << MPTX_SHIFT) | (ptx << PTX_SHIFT) | offset;
}

#define PTE_V       0x001   // Valid
#define PTE_R       0x002   // Read
#define PTE_W       0x004   // Write
#define PTE_X       0x008   // Execute
#define PTE_U       0x010   // User
#define PTE_G       0x020   // Global
#define PTE_A       0x040   // Accessed
#define PTE_D       0x080   // Dirty
#define PTE_RSW     0x300   // Reserved

#define PTE_USER    (PTE_V | PTE_R | PTE_W | PTE_X | PTE_U)

#define PTE_SHIFT   10


// struct Page - page description structures. Each Page describes one
// physical page. In kernel/mm/pmm.h, you can find lots of useful functions
// that convert Page to other data types, such as physical address
struct page {
    struct list free_list_link;     // free list link
    u32 ref;                   // page frame's reference counter
    u32 flags;                 // array of flags tha describe the status of the page frame
    u32 property;              // the num of free block, used in first fit pm manager

    // used for page replace algorithm
    struct list pra_list_link;
    u64 pra_addr;
};

// Flags describing the status of a page frame

// if this bit = 1: the page is reserved for the kernel , cannot be used in alloc/free_pages
// otherwise, this bit = 0
#define PAGE_RESERVED   0x00000001

// if this bit = 1: the page is the head of a free memory block
// (contains some continous_address pages), and can be used in alloc_pages
// if this bit = 0: the page is the head page of a free memory block,
// then this page and the memory block is alloced. Or this page isn't the head page;
#define PAGE_PROPERTY   0x00000002

#define LIST2PAGE(member, list)     \
    TO_STRUCT(struct page, member, (list))


struct free_area {
    struct list free_list;  // the list header
    u64 num_free;      // number of free pages in this free list
};


#endif // __KERNEL_MM_PAGE_H__
