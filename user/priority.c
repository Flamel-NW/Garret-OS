#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "syscall.h"
#include "assert.h"

#define TOTAL 5
/* to get enough accuracy, MAX_TIME (the running time of each process) should >1000 mseconds. */
#define MAX_TIME 10000

u32 acc[TOTAL];
u64 status[TOTAL];
i32 pids[TOTAL];

static void spin_delay(void) {
    volatile i32 j;
    for (i32 i = 0; i != 200; i++)
        j = !j;
}

i32 main(void) {
    memset(pids, 0, sizeof(pids));
    sys_set_priority(TOTAL + 1);

    i32 time;
    for (i32 i = 0; i < TOTAL; i++) {
        acc[i] = 0;
        if ((pids[i] = sys_fork()) == 0) {
            sys_set_priority(i + 1);
            acc[i] = 0;
            while (true) {
                spin_delay();
                acc[i]++;
                if (acc[i] % 4000 == 0) {
                    if ((time = sys_gettime()) > MAX_TIME) {
                        putstr("child pid "); put_i64(sys_getpid(), 10);
                        putstr(", acc "); put_i64(acc[i], 10);
                        putstr(", time "); put_i64(time, 10); putch('\n');
                        sys_exit(acc[i]);
                    }
                }
            }
        }
        if (pids[i] < 0) {
            for (i32 i = 0; i < TOTAL; i++)
                if (pids[i] > 0)
                    sys_kill(pids[i]);
            PANIC("FAIL: T.T\n");
        }
    }

    putstr("main: sys_fork ok,now need to wait pids.\n");

    for (i32 i = 0; i < TOTAL; i++) {
        status[i] = 0;
        sys_wait(pids[i], &status[i]);
        putstr("main: pid "); put_i64(pids[i], 10);
        putstr(", acc "); put_i64(status[i], 10);
        putstr(", time "); put_i64(sys_gettime(), 10); putch('\n');
    }
    putstr("main: wait pids over\n");
    putstr("stride sched correct result:");
    for (i32 i = 0; i < TOTAL; i++) {
        putch(' '); put_i64((status[i] * 2 / status[0] + 1) / 2, 10);
    }
    putch('\n');

    putstr("priority pass.\n");
    return 0;
}

