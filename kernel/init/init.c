#include "defs.h"
#include "intr.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "debug.h"
#include "timer.h"
#include "trap.h"

int32_t kernel_init() __attribute__((noreturn));

int kernel_init() {
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    putstr("\n\n---Garret-OS is loading...---\n");
    putstr("Github:\thttps://github.com/Flamel-NW/Garret-OS\n");
    putstr("Email:\tflamel.nw@qq.com\n\n");

    print_kernel_info();

    idt_init();

    timer_init();

    intr_enable();

    asm volatile ( "ebreak" :: );

    while (1)
        continue;
}
