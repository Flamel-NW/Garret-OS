#include "stdlib.h"

#include "defs.h"
#include "string.h"


const char* i64toa(i64 value, u8 base) {
    ASSERT(base >= 2);
    static char st[128];
    if (!value) {
        st[0] = '0';
        st[1] = '\0';
    } else {
        bool is_neg = value < 0;
        if (is_neg) value *= -1;
        u8 sp = 0;
        while (value) {
            st[sp++] = u8toc(value % base);
            value /= base;
        }
        if (is_neg) st[sp++] = '-';
        st[sp] = '\0';
        strrev(st);
    }
    return st;
}

const char* u64toa(u64 value, u8 base) {
    ASSERT(base >= 2);
    static char st[128];
    if (!value) {
        st[0] = '0';
        st[1] = '\0';
    } else {
        u8 sp = 0;
        while (value) {
            st[sp++] = u8toc(value % base);
            value /= base;
        }
        st[sp] = '\0';
        strrev(st);
    } 
    return st;
}

const char* u8toa(u8 value, u8 base) {
    return u64toa(value, base);
}

const char* u32toa(u32 value, u8 base) {
    return u64toa(value, base);
}

const char* i32toa(i32 value, u8 base) {
    return i64toa(value, base);
}
