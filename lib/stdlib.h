#ifndef __LIB_STDLIB_H__
#define __LIB_STDLIB_H__

#include "defs.h"

// lib/stdlib.c
static inline char u8toc(u8 value) {
    return value < 10 ? value + '0' : value - 10 + 'A';
}

bool u8toa(u8 value, u8 base, char* buf, u64 len);

bool u32toa(u32 value, u8 base, char* buf, u64 len);
bool i32toa(i32 value, u8 base, char* buf, u64 len);

bool i64toa(i64 value, u8 base, char* buf, u64 len);
bool u64toa(u64 value, u8 base, char* buf, u64 len);

// lib/hash.c

// hash32 - generate a hash value in the range [0, 2^@bits - 1]
//          High bits are more random, so we use them
u32 hash32(u32 val, u32 bits);

// lib/rand.c

/* the largest number rand will return */
#define RAND_MAX 2147483647UL

i32 rand(void);
void srand(u32 seed);

#endif // __LIB_STDIO_H__
