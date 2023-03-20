#include "stdlib.h"
#include "assert.h"
#include "defs.h"
#include "string.h"

inline char ui8toc(uint8_t value) {
    ASSERT(value - 10 + 'A' <= 'Z');
    return value < 10 ? value + '0' : value - 10 + 'A';
}

char* i32toa(int32_t value, char* buffer, uint8_t base) {
    return i64toa(value, buffer, base);
}

char* i64toa(int64_t value, char* buffer, uint8_t base) {
    if (value < 0) {
        ui64toa(value * -1, buffer + 1, base);
        buffer[0] = '-';
    } else {
        ui64toa(value, buffer, base);
    }
    return buffer;
}

char* ui64toa(uint64_t value, char* buffer, uint8_t base) {
    ASSERT(base >= 2);
    
    char st[70];
    uint8_t sp = 0;
    
    while (value) {
        st[sp++] = ui8toc(value % base);
        value /= base;
    }
    st[sp] = '\0';

    strrev(st);
    strcpy(buffer, st);

    sp = 0;
    return buffer;
}

char* ptrtoa(intptr_t value, char* buffer, uint8_t base) {
    return i64toa(value, buffer, base);
}

char* uptrtoa(uintptr_t value, char* buffer, uint8_t base) {
    return ui64toa(value, buffer, base);
}
