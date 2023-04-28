#ifndef __LIB_STDLIB_H__
#define __LIB_STDLIB_H__

#include "defs.h"
#include "assert.h"

static inline char u8toc(uint8_t value) {
    ASSERT(value - 10 + 'A' <= 'Z');
    return value < 10 ? value + '0' : value - 10 + 'A';
}

const char* u8toa(uint8_t value, uint8_t base);

const char* u32toa(uint32_t value, uint8_t base);
const char* i32toa(int32_t value, uint8_t base);

const char* i64toa(int64_t value, uint8_t base);
const char* u64toa(uint64_t value, uint8_t base);

const char* ptrtoa(intptr_t value, uint8_t base);
const char* uptrtoa(uintptr_t value, uint8_t base);

#endif // __LIB_STDIO_H__
