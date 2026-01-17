#ifndef __KERNEL_TRAP_TRAP_H__
#define __KERNEL_TRAP_TRAP_H__

#include "defs.h"

// Kernel thread syscall magic number - used to identify kernel thread syscalls via ebreak
#define EBREAK_MAGIC    0x7FF // Random magic number for ebreak-based syscalls (12-bit immediate: 2047)


// General Purpose Registers
struct gpr {
    u64 zero;     // Hard-wired zero
    u64 ra;       // Return address
    u64 sp;       // Stack pointer
    u64 gp;       // Global pointer
    u64 tp;       // Thread pointer
    u64 t0;       // Temporary
    u64 t1;       // Temporary
    u64 t2;       // Temporary
    u64 s0;       // Saved register/frame pointer
    u64 s1;       // Saved register
    u64 a0;       // Function argument/return value
    u64 a1;       // Function argument/return value
    u64 a2;       // Function argument
    u64 a3;       // Function argument
    u64 a4;       // Function argument
    u64 a5;       // Function argument
    u64 a6;       // Function argument
    u64 a7;       // Function argument
    u64 s2;       // Saved register
    u64 s3;       // Saved register
    u64 s4;       // Saved register
    u64 s5;       // Saved register
    u64 s6;       // Saved register
    u64 s7;       // Saved register
    u64 s8;       // Saved register
    u64 s9;       // Saved register
    u64 s10;      // Saved register
    u64 s11;      // Saved register
    u64 t3;       // Temporary
    u64 t4;       // Temporary
    u64 t5;       // Temporary
    u64 t6;       // Temporary
};

struct trapframe {
    struct gpr gpr;         // General Purpose Registers
    u64 sstatus;            // Supervisor Status Register
    u64 sepc;               // Supervisor Exception Program Counter
    // warning: 'sbadaddr' is a deprecated alias for 'stval'
    u64 stval;              // Supervisor Trap Value Register
    u64 scause;             // Supervisor Cause Register
};

void trap(struct trapframe* tf);
void init_idt();

void print_trapframe(struct trapframe* tf);
void print_registers(struct gpr* p_gpr);

#endif // __KERNEL_TRAP_TRAP_H__
