#include <stdlib.h>

// 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 
#define GOLDEN_RATIO_PRIME_32       0x9e370001UL


// hash32 - generate a hash value in the range [0, 2^@bits - 1]
//          High bits are more random, so we use them
u32 hash32(u32 val, u32 bits) {
    u32 hash = val * GOLDEN_RATIO_PRIME_32;
    return (hash >> (32 - bits));
}
