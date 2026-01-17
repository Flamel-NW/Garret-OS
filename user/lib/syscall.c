#include "syscall.h"
#include "stdarg.h"
#include "stdio.h"
#include "unistd.h"

#define MAX_SYSCALL_ARGS    5   //  enough for now, no special reason


static inline i32 syscall(u64 num, ...) {
    va_list ap;
    va_start(ap, num);
    u64 a[MAX_SYSCALL_ARGS];
    for (i32 i = 0; i < MAX_SYSCALL_ARGS; i++) {
        a[i] = va_arg(ap, u64);
    }
    va_end(ap);

    i32 ret;
    __asm__ volatile (
        "mv a0, %1\n"
        "mv a1, %2\n"
        "mv a2, %3\n"
        "mv a3, %4\n"
        "mv a4, %5\n"
        "mv a5, %6\n"
        "ecall\n"
        "mv %0, a0"
        : "=r" (ret)
        : "r"(num), "r"(a[0]), "r"(a[1]), "r"(a[2]), "r"(a[3]), "r"(a[4])
        : "memory", "a0", "a1", "a2", "a3", "a4", "a5");

    return ret;
}

i32 sys_yield() {
    return syscall(SYS_YIELD);
}

i32 sys_exit(u64 status) {
    i32 ret = syscall(SYS_EXIT, status);
    putstr("BUG: exit failed.\n");
    while (true)
        continue;
    return ret;
}

i32 sys_fork() {
    return syscall(SYS_FORK);
}

i32 sys_wait(i32 pid, u64* wstatus) {
    return syscall(SYS_WAIT, pid, wstatus);
}

i32 sys_putc(u64 c) {
    return syscall(SYS_PUTC, c);
}

i32 sys_gpt() {
    return syscall(SYS_GPT);
}

i32 sys_getpid() {
    return syscall(SYS_GETPID);
}

i32 sys_kill(i32 pid) {
    return syscall(SYS_KILL, pid);
}
