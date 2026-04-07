#include "stdio.h"
#include "assert.h"

i32 main() {
    putstr("I will read from 0xfac00000.\n"); 
    put_u64(*(u32* ) 0xfac00000, 16);
    PANIC("FAIL: T.T\n");
}
