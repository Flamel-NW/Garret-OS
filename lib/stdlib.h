#ifndef __LIB_STDLIB_H__
#define __LIB_STDLIB_H__

#include "defs.h"
#include "assert.h"

static inline char u8toc(u8 value) {
    ASSERT(value - 10 + 'A' <= 'Z');
    return value < 10 ? value + '0' : value - 10 + 'A';
}

const char* u8toa(u8 value, u8 base);

const char* u32toa(u32 value, u8 base);
const char* i32toa(i32 value, u8 base);

const char* i64toa(i64 value, u8 base);
const char* u64toa(u64 value, u8 base);

// lib/hash.c

// hash32 - generate a hash value in the range [0, 2^@bits - 1]
//          High bits are more random, so we use them
u32 hash32(u32 val, u32 bits);

#endif // __LIB_STDIO_H__
