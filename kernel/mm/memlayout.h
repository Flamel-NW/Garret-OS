#ifndef __KERNEL_MM_MEMLAYOUT_H__
#define __KERNEL_MM_MEMLAYOUT_H__

// 这里是相对uCore-OS的一处核心修改！

// Virtual Memory Map:                                            Permissions
//                                                                kernel/user
//
//     512G ----------------> +---------------------------------+ 0x FFFFFFFF FFFFFFFF
//                            |  Cur. Page Table (Kernel, RW)   | RW/--
//     VPT -----------------> +---------------------------------+ 0x FFFFFFFF C0000000
//                            |                                 |
//                            |         Empty Memory (*)        | 
//                            |                                 |
//                 ---------> +---------------------------------+ 0x FFFFFFFF 90000000
//                            |        Invalid Memory (*)       | --/--
//     DRAM_END ------------> +---------------------------------+ 0x FFFFFFFF 88000000
//                            |                                 |
//                 DRAM_SIZE  |    Remapped Physical Memory     | RW/--
//                            |                                 |
//     DRAM_BEGIN_VA -------> +---------------------------------+ 0x FFFFFFFF 80000000
//                            |        Invalid Memory (*)       | --/--
//     USER(_STACK)_END ----> +---------------------------------+ 0x 00000000 80000000
//                            |           User stack            |
//     USER_STACK_BEGIN ----> +---------------------------------+ 0x 00000000 7FF00000
//                            |                                 |
//                            :                                 :
//                            |         ~~~~~~~~~~~~~~~~        |
//                            :                                 :
//                            |                                 |
//                            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                            |       User Program & Heap       |
//     USER_TEXT -----------> +---------------------------------+ 0x 00000000 00800000
//                            |        Invalid Memory (*)       | --/--
//                            |  - - - - - - - - - - - - - - -  |
//                            |    User STAB Data (optional)    |
//     USER_BEGIN USER_STAB-> +---------------------------------+ 0x 00000000 00200000
//                            |        Invalid Memory (*)       | --/--
//     0 -------------------> +---------------------------------+ 0x 00000000 00000000
// (*) Note: The kernel ensures that "Invalid Memory" is *never* mapped.
//     "Empty Memory" is normally unmapped, but user programs may map pages
//     there if desired.

#define PAGE_SHIFT          12
#define PAGE_SIZE           (1 << PAGE_SHIFT)

#define KERNEL_STACK_PAGE   2
#define KERNEL_STACK_SIZE   (KERNEL_STACK_PAGE << PAGE_SHIFT)

// only valid in DRAM
#define VA_PA_OFFSET        0xFFFFFFFF00000000

// memory starts at 0x80000000 in Open-SBI 
#define DRAM_BEGIN_PA       0x80000000
#define DRAM_BEGIN_VA       (DRAM_BEGIN_PA + VA_PA_OFFSET)

#define DRAM_END_PA         0x88000000
#define DRAM_END_VA         (DRAM_END_PA + VA_PA_OFFSET)

#define KERNEL_BEGIN_PA     0x80200000
#define KERNEL_BEGIN_VA     (KERNEL_BEGIN_PA + VA_PA_OFFSET)

#define KERNEL_END_PA       0x82000000
#define KERNEL_END_VA       (KERNEL_END_PA + VA_PA_OFFSET)

// VA_PA_OFFSET is NOT valid now
#define USER_BEGIN          0x00200000
#define USER_END            0x80000000

#define USER_PAGE           256
#define USER_STACK_SIZE     (USER_PAGE * PAGE_SIZE)

#define USER_STACK_END      USER_END
#define USER_STACK_BEGIN    (USER_STACK_END - USER_STACK_SIZE)

#define USER_TEXT           0x00800000
#define USER_STAB           USER_BEGIN

#ifndef __ASSEMBLER__

#include "defs.h"
#include "assert.h"

static inline bool kernel_access(u64 begin, u64 end) {
    return KERNEL_BEGIN_VA <= begin && begin < end && end <= KERNEL_END_VA;
}

static inline bool user_access(u64 begin, u64 end) {
    return USER_BEGIN <= begin && begin < end && end <= USER_END;
}

static inline u64 pa2va(u64 pa) {
    ASSERT(pa > DRAM_BEGIN_PA && pa < DRAM_END_PA);
    return pa + VA_PA_OFFSET;
}

static inline u64 va2pa(u64 va) {
    ASSERT(va > DRAM_BEGIN_VA && va < DRAM_END_VA);
    return va - VA_PA_OFFSET;
}

#endif // __ASSEMBLER__

#endif // __KERNEL_MM_MEMLAYOUT_H__
