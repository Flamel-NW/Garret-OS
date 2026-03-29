#include "stdio.h"
#include "assert.h"
#include "syscall.h"


i32 parent;
i32 pid1, pid2;

void do_yield() {
    sys_yield();
    sys_yield();
    sys_yield();
    sys_yield();
    sys_yield();
    sys_yield();
}

void loop() {
    putstr("child 1.\n");
    while (true)
        continue;
}

void work() {
    putstr("child 2.\n");
    do_yield();
    if (sys_kill(parent) == 0) {
        putstr("waitkill parent pass.\n");
        do_yield();
        if (sys_kill(pid1) == 0) {
            putstr("waitkill child1 pass.\n");
            sys_exit(0);
        }
    }
    sys_exit(-1);
}

i32 main() {
    parent = sys_getpid();
    if ((pid1 = sys_fork()) == 0) 
        loop();
    
    ASSERT(pid1 > 0);

    if ((pid2 = sys_fork()) == 0) 
        work();
    
    if (pid2 > 0) {
        putstr("wait child 1.\n");
        sys_wait(pid1, NULL);
        putstr("pid1 = "); put_i64(pid1, 10); putch('\n');
        PANIC("sys_wait returns\n");
    }
    else {
        sys_kill(pid1);
    }
    PANIC("FAIL: T.T\n");
}