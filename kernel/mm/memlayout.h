#ifndef __KERNEL_MM_MEMLAYOUT_H__
#define __KERNEL_MM_MEMLAYOUT_H__

#define PAGE_SHIFT  12
#define PAGE_SIZE   (1 << PAGE_SHIFT)

#define KERN_STACK_PAGE 2
#define KERN_STACK_SIZE (KERN_STACK_PAGE << PAGE_SHIFT)

#define VA_PA_OFFSET    0xFFFFFFFF40000000

// memory starts at 0x80000000 in RISC-V
#define DRAM_BASE_PA    0x80000000
#define DRAM_BASE_VA    (DRAM_BASE_PA + VA_PA_OFFSET)

#define DRAM_END_PA     0x88000000
#define DRAM_END_VA     (DRAM_END_PA + VA_PA_OFFSET)

#define KERNEL_BEGIN_PA 0x80200000
#define KERNEL_BEGIN_VA (KERNEL_BEGIN_PA + VA_PA_OFFSET)

#define KERNEL_END_PA   0x82000000
#define KERNEL_END_VA   (KERNEL_END_PA + VA_PA_OFFSET)

#define FREE_BEGIN_PA   0x82000000
#define FREE_BEGIN_VA   (FREE_BEGIN_PA + VA_PA_OFFSET)

#define FREE_END_PA     0x88000000
#define FREE_END_VA     (FREE_END_PA + VA_PA_OFFSET)

#ifndef __ASSEMBLER__

#include "defs.h"

static inline uintptr_t pa2va(uintptr_t pa) {
    return pa + VA_PA_OFFSET;
}

static inline uintptr_t va2pa(uintptr_t va) {
    return va - VA_PA_OFFSET;
}

#endif // __ASSEMBLER__

#endif // __KERNEL_MM_MEMLAYOUT_H__
