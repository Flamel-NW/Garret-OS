#include "stdio.h"
#include "assert.h"

i32 main() {
    putstr("I will read from 0.\n"); 
    put_u64(*(volatile u32* ) 0, 16);
    PANIC("FAIL: T.T\n");
}
