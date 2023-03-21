#ifndef __LIB_STDLIB_H__
#define __LIB_STDLIB_H__

#include "defs.h"
#include "assert.h"

static inline char ui8toc(uint8_t value) {
    ASSERT(value - 10 + 'A' <= 'Z');
    return value < 10 ? value + '0' : value - 10 + 'A';
}

char* ui8toa(uint8_t value, uint8_t base);

char* i32toa(int32_t value, uint8_t base);

char* i64toa(int64_t value, uint8_t base);
char* ui64toa(uint64_t value, uint8_t base);

char* ptrtoa(intptr_t value, uint8_t base);

char* uptrtoa(uintptr_t value, uint8_t base);

#endif // __LIB_STDIO_H__
