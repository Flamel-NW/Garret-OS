#include "syscall.h"

i32 main(void);

void user_main(void) {
    i32 ret = main();
    sys_exit(ret);
}

