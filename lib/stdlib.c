#include "stdlib.h"

#include "defs.h"
#include "string.h"


bool i64toa(i64 value, u8 base, char* buf, u64 len) {
    if (!buf || len < 2 || base < 2 || base > 36)
        return false;

    if (!value) {
        buf[0] = '0';
        buf[1] = '\0';
        return true;
    } else {
        bool is_neg = value < 0;
        u64 mag = is_neg ? (u64)(-(value + 1)) + 1 : (u64)value;
        if (is_neg) {
            buf[0] = '-';
            return u64toa(mag, base, buf + 1, len - 1);
        } else {
            return u64toa(mag, base, buf, len);
        }
    }
}

bool u64toa(u64 value, u8 base, char* buf, u64 len) {
    if (!buf || len < 2 || base < 2 || base > 36)
        return false;

    if (!value) {
        buf[0] = '0';
        buf[1] = '\0';
    } else {
        u64 sp = 0;
        while (value && sp < len - 1) {
            buf[sp++] = u8toc(value % base);
            value /= base;
        }
        buf[sp] = '\0';
        strrev(buf);
    } 
    return !value;
}

bool u8toa(u8 value, u8 base, char* buf, u64 len) {
    return u64toa(value, base, buf, len);
}

bool u32toa(u32 value, u8 base, char* buf, u64 len) {
    return u64toa(value, base, buf, len);
}

bool i32toa(i32 value, u8 base, char* buf, u64 len) {
    return i64toa(value, base, buf, len);
}
