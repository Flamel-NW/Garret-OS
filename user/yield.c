#include "stdio.h"
#include "syscall.h"


i32 main() {
    putstr("Hello, I am process "); put_i64(sys_getpid(), 10); putstr(".\n");
    for (i32 i = 0; i < 5; i++) {
        sys_yield();
        putstr("Back in process "); put_i64(sys_getpid(), 10);
        putstr(", iteration "); put_i64(i, 10); putstr(".\n");
    }
    putstr("All done in process "); put_i64(sys_getpid(), 10); putstr(".\n");
    putstr("yield pass.\n");
    return 0;
}