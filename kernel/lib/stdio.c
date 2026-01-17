#include "stdio.h"
#include "ctype.h"
#include "defs.h"
#include "sbi.h"

void putch(char c) {
    sbi_console_putchar(c);
}

void putstr(const char* str) {
    while (*str) {
        putch(*str);
        str++;
    }
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
