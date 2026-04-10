#include "stdio.h"
#include "syscall.h"
#include "assert.h"


void sleep(i32 pid) {
    for (i32 i = 0; i < 10; i++) {
        sys_sleep(100);
        putstr("sleep "); put_i64(i + 1, 10); putstr(" * 100 slices.\n");
    }
    sys_exit(0);
}

i32 main() {
    i32 time = sys_gettime();

    i32 pid = sys_fork();
    if (!pid)
        sleep(pid);

    u64 code;
    ASSERT(sys_wait(pid, &code) == 0 && code == 0);
    putstr("use "); put_i64(sys_gettime() - time, 10); putstr(" msecs.\n");

    putstr("sleep pass.\n");
    return 0;
}
