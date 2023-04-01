#include "assert.h"

#include "defs.h"
#include "monitor.h"
#include "stdio.h"
#include "intr.h"

static bool is_panic = 0;

static void panic_dead() __attribute__((noreturn));

static void panic_dead() {
    intr_disable();
    while (1)
        monitor(NULL);
}

static void putline(uint32_t line) {
    if (!line) return;
    putline(line / 10);
    putch(line % 10 + '0');
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
