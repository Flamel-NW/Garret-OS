#include "stdio.h"
#include "syscall.h"
#include "stdlib.h"


void putch(char c) {
    sys_putc(c);
}

void putstr(const char* str) {
    while (*str) {
        putch(*str);
        str++;
    }
}

void put_u64(u64 value, u8 base) {
    char buf[128];
    if (u64toa(value, base, buf, 128))
        putstr(buf);
    else
        putstr("u64toa error\n");
}

void put_i64(i64 value, u8 base) {
    char buf[128];
    if (i64toa(value, base, buf, 128))
        putstr(buf);
    else
        putstr("i64toa error\n");
}
