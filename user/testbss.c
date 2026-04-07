#include "stdio.h"
#include "assert.h"

#define ARRAYSIZE (1024 * 1024)

u32 bigarray[ARRAYSIZE];

i32 main() {
    putstr("Making sure bss works right...\n");

    for (i32 i = 0; i < ARRAYSIZE; i++)
        ASSERT(bigarray[i] == 0);

    for (i32 i = 0; i < ARRAYSIZE; i++)
        bigarray[i] = i;

    for (i32 i = 0; i < ARRAYSIZE; i++)
        ASSERT(bigarray[i] == i);

    putstr("Yes, good.  Now doing a wild write off the end...\n");
    putstr("testbss pass.\n");

    volatile u32 off = 1024;
    bigarray[ARRAYSIZE + off] = 0;
    PANIC("FAIL: T.T\n");
}
