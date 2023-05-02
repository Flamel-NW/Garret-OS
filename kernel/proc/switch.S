#include "riscv.h"

    .text

    # void switch_proc(struct context* from, struct context* to);
    .globl switch_proc
switch_proc:
    # save from's registers
    sd ra, 0 * REGBYTES(a0)
    sd sp, 1 * REGBYTES(a0)
    sd s0, 2 * REGBYTES(a0)
    sd s1, 3 * REGBYTES(a0)
    sd s2, 4 * REGBYTES(a0)
    sd s3, 5 * REGBYTES(a0)
    sd s4, 6 * REGBYTES(a0)
    sd s5, 7 * REGBYTES(a0)
    sd s6, 8 * REGBYTES(a0)
    sd s7, 9 * REGBYTES(a0)
    sd s8, 10 * REGBYTES(a0)
    sd s9, 11 * REGBYTES(a0)
    sd s10, 12 * REGBYTES(a0)
    sd s11, 13 * REGBYTES(a0)

    # restore to's registers
    ld ra, 0 * REGBYTES(a1)
    ld sp, 1 * REGBYTES(a1)
    ld s0, 2 * REGBYTES(a1)
    ld s1, 3 * REGBYTES(a1)
    ld s2, 4 * REGBYTES(a1)
    ld s3, 5 * REGBYTES(a1)
    ld s4, 6 * REGBYTES(a1)
    ld s5, 7 * REGBYTES(a1)
    ld s6, 8 * REGBYTES(a1)
    ld s7, 9 * REGBYTES(a1)
    ld s8, 10 * REGBYTES(a1)
    ld s9, 11 * REGBYTES(a1)
    ld s10, 12 * REGBYTES(a1)
    ld s11, 13 * REGBYTES(a1)

    ret
