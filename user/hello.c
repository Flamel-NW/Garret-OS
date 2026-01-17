#include "stdio.h"
#include "defs.h"
#include "stdlib.h"
#include "syscall.h"

i32 main() {
    putstr("Hello world!.\n");
    putstr("I am process "); putstr(i32toa(sys_getpid(), 10));
    putstr(".\nhello pass.\n");
    return 0;
}
