#include "syscall.h"
#include "stdio.h"
#include "string.h"

#define DEPTH 4

void forktree(const char* cur);

void forkchild(const char* cur, char branch) {
    char next[DEPTH + 1];

    i32 len = strlen(cur);
    if (len >= DEPTH)
        return;

    strcpy(next, cur);
    next[len] = branch;
    next[len + 1] = '\0';

    if (!sys_fork()) {
        forktree(next);
        sys_yield();
        sys_exit(0);
    }
}

void forktree(const char* cur) {
    putstr("PID = "); put_i64(sys_getpid(), 10);
    putstr(": I am '"); putstr(cur); putstr("'\n");

    forkchild(cur, '0');
    forkchild(cur, '1');
}

i32 main() {
    forktree("");
    putstr("forktree pass.\n");
    return 0;
}

