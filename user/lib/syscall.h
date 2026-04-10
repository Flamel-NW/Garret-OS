#ifndef __USER_LIB_SYSCALL_H__
#define __USER_LIB_SYSCALL_H__

#include "defs.h"

i32 sys_yield();
i32 sys_exit(u64 status);
i32 sys_fork();
i32 sys_wait(i32 pid, u64* wait_status);
i32 sys_putc(u64 c);
i32 sys_getpid();
i32 sys_kill(i32 pid);
i32 sys_gettime(void);
void sys_set_priority(u64 priority);
i32 sys_sleep(u64 time);

#endif // __USER_LIB_SYSCALL_H__