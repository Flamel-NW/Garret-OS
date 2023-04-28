#include "assert.h"

#include "defs.h"
#include "stdio.h"
#include "intr.h"
#include "string.h"

static bool is_panic = 0;

static void panic_dead() __attribute__((noreturn));

static void panic_dead() {
    intr_disable();
    while (1)
        continue;;
}

static void putline(uint32_t line) {
    if (!line) return;
    static char st[128];
    if (!line) {
        st[0] = '0';
        st[1] = '\0';
    } else {
        uint8_t sp = 0;
        while (line) {
            st[sp++] = line % 10 + '0';
            line /= 10;
        }
        st[sp] = '\0';
        strrev(st);
    } 
    putstr(st);
}

void warn(const char* file, const char* func, int32_t line, const char* str) {
    putstr("kernel warning at file: ");
    putstr(file);
    putstr(", func: ");
    putstr(func);
    putstr(", line: ");
    putline(line);
    putstr(".\n");
    putstr(str);
    putch('\n');
    putch('\n');
}

void panic(const char* file, const char* func, int32_t line, const char* str) {
    if (!is_panic) {
        putstr("kernel panic at file: ");
        putstr(file);
        putstr(", func: ");
        putstr(func);
        putstr(", line: ");
        putline(line);
        putstr(".\n");
        putstr(str);
        putch('\n');
        putch('\n');
    } 
    panic_dead();
}
