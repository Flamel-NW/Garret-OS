#ifndef __KERNEL_DRIVER_SBI_H__
#define __KERNEL_DRIVER_SBI_H__

#include "defs.h"


void sbi_console_putchar(u64 ch);

u64 sbi_console_getchar();

void sbi_set_timer(u64 stime);

void sbi_shutdown();

void sbi_clear_ipi();

void sbi_send_ipi(const u64* hart_mask);

void sbi_remote_fenci_i(const u64* hart_mask);

void sbi_remote_sfence_vma(const u64* hart_mask);

void sbi_remote_sfence_vma_asid(const u64* hart_mask);


#endif // __KERNEL_DRIVER_SBI_H__
