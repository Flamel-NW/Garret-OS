#include "memlayout.h"

# 必须写.text.kernel_entry不能写.text, 与kernel.ld中保持一致, 否则不会把kernel_entry放到最前面
    .section .text.kernel_entry, "ax", %progbits
    .global kernel_entry
kernel_entry:
    # t0的高20位 := 三级页表的虚拟地址的高20位 即虚拟页号
    lui t0, %hi(boot_page_table_sv39)
    # Zero-extend word: 将低 32 位零扩展到 64 位 相当于减去虚实映射偏移量 0x FFFFFFFF 00000000
    zext.w t0, t0     
    # t0 >>= 12, 变为三级页表的物理页号
    srli t0, t0, 12

    # t1 := 8 << 60, 设置satp的MODE字段为Sv39
    li t1, 8 << 60
    # 将刚才计算出的预设三级页表物理页号附加到satp中
    or t0, t0, t1
    # 将算出的t0(即MODE|页表基址物理页号)覆盖到satp中
    csrw satp, t0

    sfence.vma # 刷新TLB

    # 从此, 我们给内核搭建出了一个完美的虚拟内存空间!

    # la sp, boot_stack_top
    # tail init_kernel
    # 我们在虚拟内存空间中: 随意将sp设置为虚拟地址!
    lui sp, %hi(boot_stack_top)

    # 我们在虚拟内存空间中: 随意跳转到虚拟地址
    # 跳转到init_kernel
    lui t0, %hi(init_kernel)
    addi t0, t0, %lo(init_kernel)
    jr t0

    .section .data
    .align PAGE_SHIFT
    .global boot_stack    
boot_stack:
    .space KERNEL_STACK_SIZE 
    .global boot_stack_top
boot_stack_top:

    .section .data
    .align PAGE_SHIFT
    .global boot_page_table_sv39
# 分配 4KB 内存给预设的三级页表
boot_page_table_sv39:
    .zero 8 * 2
    # 0x80000000 map to 0x80000000 (1G)
    .quad (0x80000 << 10) | 0xcf # VRWXD
    # 中间507个页表项设置为0, 因此 V = 0, 意味着是空的(unmapped) 
    .zero 8 * 507
    # 0x FFFFFFFF 80000000 map to 0x 00000000 80000000 (1G)
    # 设置最后一个页表项, PPN = 0x80000, 标志位VRWXD均为1
    .quad (0x80000 << 10) | 0xcf # VRWXD
