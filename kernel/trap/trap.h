#ifndef __KERNEL_TRAP_TRAP_H__
#define __KERNEL_TRAP_TRAP_H__

#include "defs.h"

struct registers {
    uintptr_t zero;     // Hard-wired zero
    uintptr_t ra;       // Return address
    uintptr_t sp;       // Stack pointer
    uintptr_t gp;       // Global pointer
    uintptr_t tp;       // Thread pointer
    uintptr_t t0;       // Temporary
    uintptr_t t1;       // Temporary
    uintptr_t t2;       // Temporary
    uintptr_t s0;       // Saved register/frame pointer
    uintptr_t s1;       // Saved register
    uintptr_t a0;       // Function argument/return value
    uintptr_t a1;       // Function argument/return value
    uintptr_t a2;       // Function argument
    uintptr_t a3;       // Function argument
    uintptr_t a4;       // Function argument
    uintptr_t a5;       // Function argument
    uintptr_t a6;       // Function argument
    uintptr_t a7;       // Function argument
    uintptr_t s2;       // Saved register
    uintptr_t s3;       // Saved register
    uintptr_t s4;       // Saved register
    uintptr_t s5;       // Saved register
    uintptr_t s6;       // Saved register
    uintptr_t s7;       // Saved register
    uintptr_t s8;       // Saved register
    uintptr_t s9;       // Saved register
    uintptr_t s10;      // Saved register
    uintptr_t s11;      // Saved register
    uintptr_t t3;       // Temporary
    uintptr_t t4;       // Temporary
    uintptr_t t5;       // Temporary
    uintptr_t t6;       // Temporary
};

struct trapframe {
    struct registers regs;      // General registers
    uintptr_t sstatus;          // Supervisor Status Register
    uintptr_t sepc;             // Supervisor Exception Program Counter
    // warning: 'sbadaddr' is a deprecated alias for 'stval'
    uintptr_t stval;            // Supervisor Trap Value Register
    uintptr_t scause;           // Supervisor Cause Register
};

void trap(struct trapframe* tf);
void init_idt();

void print_trapframe(struct trapframe* tf);
void print_registers(struct registers* regs);

#endif // __KERNEL_TRAP_TRAP_H__
