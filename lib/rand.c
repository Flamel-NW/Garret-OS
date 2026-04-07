#include "stdlib.h"

static u64 next = 1;

// rand - returns a pseudo-random integer
// The rand() function return a value in the range [0, RAND_MAX].
i32 rand(void) {
    next = (next * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
    u64 result = (next >> 12);
    return result % (RAND_MAX + 1);
}

void srand(u32 seed) {
    next = seed;
}
