#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "syscall.h"
#include "assert.h"

#define MATSIZE 10
#define TOTAL   21

static i32 mata[MATSIZE][MATSIZE];
static i32 matb[MATSIZE][MATSIZE];
static i32 matc[MATSIZE][MATSIZE];

void work(u32 times) {
    for (i32 i = 0; i < MATSIZE; i++)
        for (i32 j = 0; j < MATSIZE; j++)
            mata[i][j] = matb[i][j] = 1;

    sys_yield();

    putstr("pid "); put_i64(sys_getpid(), 10);
    putstr(" is running ("); put_i64(times, 10);
    putstr(" times)!.\n");

    while (times -- > 0) {
        for (i32 i = 0; i < MATSIZE; i++) {
            for (i32 j = 0; j < MATSIZE; j++) {
                matc[i][j] = 0;
                for (i32 k = 0; k < MATSIZE; k++) {
                    matc[i][j] += mata[i][k] * matb[k][j];
                }
            }
        }
        for (i32 i = 0; i < MATSIZE; i++)
            for (i32 j = 0; j < MATSIZE; j++)
                mata[i][j] = matb[i][j] = matc[i][j];
    }
    putstr("pid "); put_i64(sys_getpid(), 10); putstr(" done!.\n");
    sys_exit(0);
}

static void kill_children(i32 pids[]) {
    for (i32 i = 0; i < TOTAL; i++)
        if (pids[i] > 0)
            sys_kill(pids[i]);
}

i32 main(void) {
    i32 pids[TOTAL];
    memset(pids, 0, sizeof(pids));

    for (i32 i = 0; i < TOTAL; i++) {
        if ((pids[i] = sys_fork()) == 0) {
            srand(i * i);
            i32 times = (((u32) rand()) % TOTAL);
            times = (times * times + 10) * 100;
            work(times);
        }
        if (pids[i] < 0) {
            kill_children(pids);
            PANIC("FAIL: T.T\n");
        }
    }

    putstr("sys_fork ok.\n");

    for (i32 i = 0; i < TOTAL; i++) {
        if (sys_wait(0, NULL) != 0) {
            putstr("sys_wait failed.\n");
            kill_children(pids);
            PANIC("FAIL: T.T\n");
        }
    }

    putstr("matrix pass.\n");
    return 0;
}

