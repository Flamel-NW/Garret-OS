#include "syscall.h"
#include "stdio.h"
#include "assert.h"

#define MAX_CHILD 32

i32 main() {
    i32 pid;
    i32 n;
    for (n = 0; n < MAX_CHILD; n++) {
        if ((pid = sys_fork()) == 0) {
            putstr("I am child "); put_i64(n, 10); putch('\n');
            sys_exit(0);
        }
        ASSERT(pid > 0);
    }

    if (n > MAX_CHILD) {
        PANIC("fork claimed to work "); put_i64(n, 10); putstr("times!\n");
    }

    for ( ; n > 0; n--) 
        if (sys_wait(0, NULL) != 0)
            PANIC("wait stopped early\n");

    if (sys_wait(0, NULL) == 0)
        PANIC("wait got too many\n");

    putstr("forktest pass.\n");
    return 0;
}