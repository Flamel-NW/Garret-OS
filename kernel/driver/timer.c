#include "timer.h"

#include "defs.h"
#include "riscv.h"
#include "stdio.h"
#include "sbi.h"


volatile size_t ticks;

static uint64_t timebase = 100000;

static uint64_t get_cycles() {
    uint64_t n;
    asm volatile (
        "rdtime %0"
        : "=r" (n)
    );
    return n;
}

void timer_init() {
    // enable timer interrupt in sie
    CSRRS(sie, SIR_STI);

    timer_next();

    // initialized timer counter 'ticks' to zero
    ticks = 0;

    putstr("++ setup timer interrupts\n");
}

void timer_next() {
    sbi_set_timer(get_cycles() + timebase);
}
