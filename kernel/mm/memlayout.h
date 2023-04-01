#ifndef __KERNEL_MM_MEMLAYOUT_H__
#define __KERNEL_MM_MEMLAYOUT_H__

#include "mmu.h"

#define KERN_STACK_PAGE 2
#define KERN_STACK_SIZE (KERN_STACK_PAGE * PAGE_SIZE)

#define VA_PA_OFFSET    0xFFFFFFFF40000000

// memory starts at 0x80000000 in RISC-V
#define DRAM_BASE_PA    0x80000000
#define DRAM_BASE_VA    0xFFFFFFFFC0000000

#define DRAM_END_PA     0x88000000
#define DRAM_END_VA     0xFFFFFFFFC8000000

#define KERNEL_BEGIN_PA 0x80200000
#define KERNEL_BEGIN_VA 0xFFFFFFFFC0200000

#define KERNEL_END_PA   0x81000000
#define KERNEL_END_VA   0xFFFFFFFFC1000000

#define FREE_BEGIN_PA   0x81000000
#define FREE_BEGIN_VA   0xFFFFFFFFC1000000

#define FREE_END_PA     0x88000000
#define FREE_END_VA     0xFFFFFFFFC8000000

#define FREE_MEMORY     (FREE_END_PA - FREE_BEGIN_PA)
#define FREE_PAGES      (FREE_MEMORY / PAGE_SIZE)

// Flags describing the status of a page frame

// if this bit = 1: the page is reserved for the kernel , cannot be used in alloc/free_pages
// otherwise, this bit = 0
#define PAGE_RESERVED   0   

// if this bit = 1: the page is the head of a free memory block
// (contains some continous_address pages), and can be used in alloc_pages
// if this bit = 0: the page is the head page of a free memory block,
// then this page and the memory block is alloced. Or this page isn't the head page;
#define PAGE_PROPERTY   1

#ifndef __ASSEMBLER__

#include "atomic.h"
#include "defs.h"
#include "list.h"

// struct Page - page description structures. Each Page describes one
// physical page. In kernel/mm/pmm.h, you can find lots of useful functions
// that convert Page to other data types, such as physical address
struct page {
    uint32_t ref;           // page frame's reference counter
    uint64_t flags;         // array of flags tha describe the status of the page frame
    uint32_t property;      // the num of free block, used in first fir pm manager
    struct list list_link;  // free list link
};

static inline void set_page_reserved(struct page* page) {
    set_bit(&page->flags, PAGE_RESERVED);
}

static inline void clear_page_reserved(struct page* page) {
    clear_bit(&page->flags, PAGE_RESERVED);
}

static inline bool test_page_reserved(struct page* page) {
    return test_bit(&page->flags, PAGE_RESERVED);
}

static inline void set_page_property(struct page* page) {
    set_bit(&page->flags, PAGE_PROPERTY);
}

static inline void clear_page_property(struct page* page) {
    clear_bit(&page->flags, PAGE_PROPERTY);
}

static inline bool test_page_property(struct page* page) {
    return test_bit(&page->flags, PAGE_PROPERTY);
}

static inline struct page* list2page(struct list* list) {
    return to_struct(struct page, list_link, list);
}

struct free_area {
    struct list free_list;  // the list header
    uint32_t num_free;      // number of free pages in this free list
};

#endif // __ASSEMBLER__

#endif // __KERNEL_MM_MEMLAYOUT_H__
