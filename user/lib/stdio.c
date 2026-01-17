#include "stdio.h"
#include "syscall.h"


void putch(char c) {
    sys_putc(c);
}

void putstr(const char* str) {
    while (*str) {
        putch(*str);
        str++;
    }
}
