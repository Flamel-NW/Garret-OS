#ifndef __KERNEL_DRIVER_SBI_H__
#define __KERNEL_DRIVER_SBI_H__

#include "defs.h"

void sbi_console_putchar(uint64_t ch);

uint64_t sbi_console_getchar();

void sbi_set_timer(uint64_t time);

#endif // __KERNEL_DRIVER_SBI_H__
