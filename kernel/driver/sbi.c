#include "sbi.h"

u16 SBI_SET_TIMER = 0;
u16 SBI_CONSOLE_PUTCHAR = 1;
u16 SBI_CONSOLE_GETCHAR = 2;
u16 SBI_CLEAR_IPI = 3;
u16 SBI_SEND_IPI = 4;
u16 SBI_REMOTE_FENCE_I = 5;
u16 SBI_REMOTE_SFENCE_VMA = 6;
u16 SBI_REMOTE_SFENCE_VMA_ASID = 7;
u16 SBI_SHUTDOWN = 8;

// __asm__ volatile(
// 	汇编指令列表
// 	∶输出操作数 //非必需
// 	∶输入操作数 //非必需
// 	∶可能影响的寄存器或存储器 //非必需
// );

static u64 sbi_call(u64 sbi_type, u64 arg0, u64 arg1, u64 arg2) {
    register u64 a0 __asm__ ("a0") = (u64)(arg0);
	register u64 a1 __asm__ ("a1") = (u64)(arg1);
	register u64 a2 __asm__ ("a2") = (u64)(arg2);
	register u64 a7 __asm__ ("a7") = (u64)(sbi_type);
	__asm__ volatile ("ecall"
		      : "+r" (a0)
		      : "r" (a1), "r" (a2), "r" (a7)
		      : "memory");
    return a0;
}

void sbi_console_putchar(u64 ch) {
    sbi_call(SBI_CONSOLE_PUTCHAR, ch, 0, 0);
}

u64 sbi_console_getchar() {
    return sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0);
}

void sbi_set_timer(u64 stime) {
    sbi_call(SBI_SET_TIMER, stime, 0, 0);
}

void sbi_shutdown() {
	sbi_call(SBI_SHUTDOWN, 0, 0, 0);
}

void sbi_clear_ipi() {
	sbi_call(SBI_CLEAR_IPI, 0, 0, 0);
}

void sbi_send_ipi(const u64* hart_mask) {
	sbi_call(SBI_SEND_IPI, (u64) hart_mask, 0, 0);
}

void sbi_remote_fenci_i(const u64* hart_mask) {
	sbi_call(SBI_REMOTE_FENCE_I, (u64) hart_mask, 0, 0);
}

void sbi_remote_sfence_vma(const u64* hart_mask) {
	sbi_call(SBI_REMOTE_SFENCE_VMA, (u64) hart_mask, 0, 0);
}

void sbi_remote_sfence_vma_asid(const u64* hart_mask) {
	sbi_call(SBI_REMOTE_SFENCE_VMA_ASID, (u64) hart_mask, 0, 0);
}
