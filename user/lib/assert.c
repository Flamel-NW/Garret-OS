#include "assert.h"

#include "defs.h"
#include "stdio.h"
#include "syscall.h"
#include "errno.h"
#include "string.h"


static bool is_panic = false;

static void panic_dead() __attribute__((noreturn));

static void panic_dead() {
    sys_exit(-E_PANIC);
    while (true)
        continue;
}


void warn(const char* file, const char* func, i32 line, const char* str) {
    putstr("user warning at file: ");
    putstr(file);
    putstr(", func: ");
    putstr(func);
    putstr(", line: ");
    put_u64(line, 10);
    putstr(".\n");
    putstr(str);
    putch('\n');
    putch('\n');
}

void panic(const char* file, const char* func, i32 line, const char* str) {
    if (!is_panic) {
        putstr("user panic at file: ");
        putstr(file);
        putstr(", func: ");
        putstr(func);
        putstr(", line: ");
        put_u64(line, 10);
        putstr(".\n");
        putstr(str);
        putch('\n');
        putch('\n');
    } 
    panic_dead();
}
