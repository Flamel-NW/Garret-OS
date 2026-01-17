#ifndef __USER_LIB_SYSCALL_H__
#define __USER_LIB_SYSCALL_H__

#include "defs.h"

i32 sys_yield();
i32 sys_exit(u64 status);
i32 sys_fork();
i32 sys_wait(i32 pid, u64* wstatus);
i32 sys_putc(u64 c);
i32 sys_gpt();
i32 sys_getpid();
i32 sys_kill(i32 pid);

#endif // __USER_LIB_SYSCALL_H__