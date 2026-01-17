#include "timer.h"

#include "defs.h"
#include "riscv.h"
#include "stdio.h"
#include "sbi.h"


volatile u64 g_ticks;

static u64 timebase = 100000;

static u64 get_cycles() {
    u64 n;
    __asm__ volatile (
        "rdtime %0"
        : "=r" (n)
    );
    return n;
}

void init_timer() {
    // enable timer interrupt in sie
    CSRRS(sie, SIR_STI);

    timer_next();

    // initialized timer counter 'g_ticks' to zero
    g_ticks = 0;

    putstr("setup timer interrupts\n\n");
}

void timer_next() {
    sbi_set_timer(get_cycles() + timebase);
}
