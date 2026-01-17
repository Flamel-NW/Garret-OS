#include "syscall.h"
#include "stdio.h"
#include "assert.h"
#include "stdlib.h"

u64 magic = 0x10384;

i32 main(void) {
    i32 pid;
    putstr("I am the parent. Forking the child...\n");
    if ((pid = sys_fork()) == 0) {
        putstr("I am the child.\n");
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_exit(magic);
    }
    else {
        putstr("I am parent, fork a child pid "); putstr(i32toa(pid, 10)); putstr("\n");
    }
    ASSERT(pid > 0);
    putstr("I am the parent, waiting now..\n");

    u64 code = 0;
    ASSERT(sys_wait(pid, &code) == 0 && code == magic);
    ASSERT(sys_wait(pid, &code) != 0 && sys_wait(0, NULL) != 0);
    putstr("waitpid "); putstr(i32toa(pid, 10)); putstr(" ok.\n");

    putstr("sys_exit pass.\n");
    return 0;
}
