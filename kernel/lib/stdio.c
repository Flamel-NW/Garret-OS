#include "stdio.h"
#include "ctype.h"
#include "defs.h"
#include "sbi.h"
#include "stdlib.h"

void putch(char c) {
    sbi_console_putchar(c);
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


char getch() {
    char ch;
    while ((ch = sbi_console_getchar()) > 127)
        continue;
    return ch; 
}

char* getstr(char* str) {
    u32 sp = 0;
    char ch;
    while ((ch = getch())) {
        putch(ch);
        if (ch == '\b' && sp > 0) sp--;
        else if (ch == '\n' || ch == '\r') break;
        else str[sp++] = ch;
    }
    str[sp] = '\0';
    return str;
}
