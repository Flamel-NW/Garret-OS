#include "stdio.h"
#include "assert.h"
#include "syscall.h"

u64 magic = 0xbeaf;

i32 main() {
    i32 pid;
    u64 code;

    if ((pid = sys_fork()) == 0) {
        putstr("fork ok.\n");
        for (i32 i = 0; i < 10; i++) {
            sys_yield();
        }
        sys_exit(magic);
    }
    ASSERT(pid > 0);
    ASSERT(sys_wait(-1, NULL) != 0);
    ASSERT(sys_wait(pid, (void*) 0xC0000000) != 0);
    ASSERT(sys_wait(pid, &code) == 0 && code == magic);

    putstr("badarg pass.\n");
    return 0;
}