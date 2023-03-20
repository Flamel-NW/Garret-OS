#include "memlayout.h"

    .section .text, "ax", %progbits
    .global kernel_entry
kernel_entry:
    la sp, boot_stack_top
    tail kernel_init

    .section .data
    .align PAGE_SHIFT
    .global boot_stack    
boot_stack:
    .space KERN_STACK_SIZE 
    .global boot_stack_top
boot_stack_top:
