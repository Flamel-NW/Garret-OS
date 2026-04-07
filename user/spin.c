#include "syscall.h"
#include "stdio.h"
#include "assert.h"


i32 main() {
    i32 pid;
    putstr("I am the parent. Forking the child...\n");
    if (!(pid = sys_fork())) {
        putstr("I am the child. spinning ...\n");
        while (true)
            continue;
    }
    putstr("I am the parent. Running the child...\n");

    sys_yield();
    sys_yield();
    sys_yield();

    putstr("I am the parent.  Killing the child...\n");

    i32 ret;
    ASSERT((ret = sys_kill(pid)) == 0);
    putstr("sys_kill returns "); put_i64(ret, 10); putch('\n');

    ASSERT((ret = sys_wait(pid, NULL)) == 0);
    putstr("wait returns "); put_i64(ret, 10); putch('\n');

    putstr("spin pass.\n");
    return 0;
}
