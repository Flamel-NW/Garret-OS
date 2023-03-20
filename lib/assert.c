#include "assert.h"

#include "defs.h"
#include "stdio.h"
#include "intr.h"

static int8_t is_panic = 0;

void panic_dead() __attribute__((noreturn));

void panic_dead() {
    intr_disable();
    while (1)
        continue;
}

static void putline(uint32_t line) {
    if (!line) return;
    putline(line / 10);
    putch(line % 10 + '0');
}

void warn(const char* file, int32_t line, const char* str) {
    putstr("kernel warning at file: ");
    putstr(file);
    putstr(", line: ");
    putline(__LINE__);
    putstr(".\n");
    putstr(str);
    putch('\n');
}

void panic(const char* file, int32_t line, const char* str) {
    if (!is_panic) {
        putstr("kernel panic at file: ");
        putstr(file);
        putstr(", line: ");
        putline(__LINE__);
        putstr(".\n");
        putstr(str);
        putch('\n');
    } 
    panic_dead();
}
