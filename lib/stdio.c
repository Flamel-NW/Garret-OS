#include "stdio.h"

void putch(char c) {
    sbi_console_putchar(c);
}

void putstr(const char* str) {
    while (*str) {
        putch(*str);
        str++;
    }
}
