#include "stdlib.h"

#include "defs.h"
#include "string.h"

const char* i64toa(int64_t value, uint8_t base) {
    ASSERT(base >= 2);
    static char st[128];
    if (!value) {
        st[0] = '0';
        st[1] = '\0';
    } else {
        bool neg = value < 0;
        if (neg) value *= -1;
        uint8_t sp = 0;
        while (value) {
            st[sp++] = u8toc(value % base);
            value /= base;
        }
        if (neg) st[sp++] = '-';
        st[sp] = '\0';
        strrev(st);
    }
    return st;
}

const char* u64toa(uint64_t value, uint8_t base) {
    ASSERT(base >= 2);
    static char st[128];
    if (!value) {
        st[0] = '0';
        st[1] = '\0';
    } else {
        uint8_t sp = 0;
        while (value) {
            st[sp++] = u8toc(value % base);
            value /= base;
        }
        st[sp] = '\0';
        strrev(st);
    } 
    return st;
}

const char* u8toa(uint8_t value, uint8_t base) {
    return u64toa(value, base);
}

const char* u32toa(uint32_t value, uint8_t base) {
    return u64toa(value, base);
}

const char* i32toa(int32_t value, uint8_t base) {
    return i64toa(value, base);
}

const char* ptrtoa(intptr_t value, uint8_t base) {
    return i64toa(value, base);
}

const char* uptrtoa(uintptr_t value, uint8_t base) {
    return u64toa(value, base);
}
