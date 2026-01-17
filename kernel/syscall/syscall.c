#include "defs.h"
#include "proc.h"
#include "trap.h"
#include "unistd.h"


static i32 sys_exit(u64 arg[]) {
    i32 errno = (i32) arg[0];
    return do_exit(errno);
}

static i32 sys_exec(u64 arg[]) {
    const char* name = (const char*) arg[0];
    u64 len = arg[1];
    u8* bin = (u8*) arg[2];
    u64 size = arg[3];
    return do_execve(name, len, bin, size);
}

static i32 sys_fork(u64 arg[]) {
    struct trapframe* tf = g_curr_proc->tf;
    u64 stack = tf->gpr.sp;
    return do_fork(stack, tf, 0);
}

static i32 sys_wait(u64 arg[]) {
    i32 pid = (i32) arg[0];
    i32* store = (i32*) arg[1];
    return do_wait(pid, store);
}

static i32 sys_yield(u64 arg[]) {
    return do_yield();
}

static i32 sys_kill(u64 arg[]) {
    i32 pid = (i32) arg[0];
    return do_kill(pid);
}

static i32 sys_getpid(u64 argp[]) {
    return g_curr_proc->pid;
}

static i32 sys_putc(u64 arg[]) {
    i32 c = (i32) arg[0];
    putch(c);
    return 0;
}

static i32 sys_gpt(u64 arg[]) {
    return 0;
}

static i32 (*syscalls[]) (u64 arg[]) = {
    [SYS_EXIT]      = sys_exit,
    [SYS_FORK]      = sys_fork,
    [SYS_WAIT]      = sys_wait,
    [SYS_EXEC]      = sys_exec,
    [SYS_YIELD]     = sys_yield,
    [SYS_KILL]      = sys_kill,
    [SYS_GETPID]    = sys_getpid,
    [SYS_PUTC]      = sys_putc,
    [SYS_GPT]       = sys_gpt
};

void syscall() {
    struct trapframe* tf = g_curr_proc->tf;
    u64 arg[5];
    i32 num = tf->gpr.a0;
    i32 num_syscalls = sizeof(syscalls) / sizeof(syscalls[0]);
    if (num >= 0 && num < num_syscalls) {
        if (syscalls[num]) {
            arg[0] = tf->gpr.a1;
            arg[1] = tf->gpr.a2;
            arg[2] = tf->gpr.a3;
            arg[3] = tf->gpr.a4;
            arg[4] = tf->gpr.a5;
            tf->gpr.a0 = syscalls[num](arg);
            return;
        }
    }
    print_trapframe(tf);
    putstr("undifined syscall "); putstr(i32toa(num, 10));
    putstr(", pid = "); putstr(i32toa(g_curr_proc->pid, 10));
    putstr(", name = "); putstr(g_curr_proc->name); putstr(".\n");
    PANIC("");
}
