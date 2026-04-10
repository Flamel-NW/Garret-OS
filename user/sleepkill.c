#include "stdio.h"
#include "syscall.h"
#include "assert.h"

i32 main() {
    i32 pid = sys_fork();
    if (!pid) {
        sys_sleep(~0);
        PANIC("FAIL: T.T\n");
    }
    ASSERT(pid > 0);
    sys_sleep(100);
    ASSERT(sys_kill(pid) == 0);
    putstr("sleepkill pass.\n");
    return 0;
}
