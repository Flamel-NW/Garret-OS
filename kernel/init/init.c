#include "defs.h"
#include "intr.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "debug.h"
#include "swap.h"
#include "timer.h"
#include "trap.h"
#include "pmm.h"
#include "vmm.h"
#include "proc.h"

int32_t init_kernel() __attribute__((noreturn));

int32_t init_kernel() {
    // 清bss段
    extern byte edata[], end[];
    memset(edata, 0, end - edata);

    putstr("\n\n---Garret-OS is loading...---\n");
    putstr("Github:\thttps://github.com/Flamel-NW/Garret-OS\n");
    putstr("Email:\tflamel.nw@qq.com\n\n\n");

    print_memory_info();
    print_kernel_info();

    init_pmm();
    print_pages_info();

    init_idt();

    init_swap();
    check_vmm();

    init_proc();

    init_timer();

    // V I: P27
    __asm__ volatile ( "ebreak" :: );

    run_idle();
}
