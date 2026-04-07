#include "syscall.h"
#include "stdio.h"
#include "assert.h"

#define MAGIC 0x10384

i32 main() {
    i32 pid;
    putstr("I am the parent. Forking the child...\n");
    if (!(pid = sys_fork())) {
        putstr("I am the child.\n");
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_yield();
        sys_exit(MAGIC);
    }
    else {
        putstr("I am parent, fork a child pid "); put_i64(pid, 10); putstr("\n");
    }
    ASSERT(pid > 0);
    putstr("I am the parent, waiting now..\n");

    u64 code = 0;
    ASSERT(sys_wait(pid, &code) == 0 && code == MAGIC);
    ASSERT(sys_wait(pid, &code) != 0 && sys_wait(0, NULL) != 0);
    putstr("waitpid "); put_i64(pid, 10); putstr(" ok.\n");

    putstr("exit pass.\n");
    return 0;
}
