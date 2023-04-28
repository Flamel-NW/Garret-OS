#include "trap.h"

#include "assert.h"
#include "defs.h"
#include "riscv.h"
#include "stdio.h"
#include "stdlib.h"
#include "timer.h"
#include "vmm.h"
#include "intr.h"

#define TICKS_NUM 100

static inline void print_ticks() {
    putstr(u8toa(TICKS_NUM, 10)); putstr(" ticks\n");
}

static int32_t page_fault_handler(struct trapframe* tf) {
    putstr("paged fault at 0x"); putstr(uptrtoa(tf->stval, 16)); putch('\n');
    if (g_check_vmm) 
        return handle_page_fault(g_check_vmm, tf->stval);
    PANIC("unhandled page fault!");
}

static void interrupt_handler(struct trapframe* tf) {
    switch (tf->scause) {
        case SCAUSE_SSI:
            putstr("Supervisor software interrupt\n");
            break;
        case SCAUSE_STI:
            // putstr("Supervisor timer interrupt\n");
            timer_next();
            if (!(++g_ticks % TICKS_NUM))
                print_ticks();
            break;
        case SCAUSE_SEI:
            putstr("Supervisor external interrupt\n");
            break;
        default:
            print_trapframe(tf);
            break;
    }
}

static void exception_handler(struct trapframe* tf) {
    int32_t errno;
    switch (tf->scause) {
        case SCAUSE_IAM:
            putstr("Instruction address misaligned\n");
            break;
        case SCAUSE_IAF:
            putstr("Instruction access fault\n");
            break;
        case SCAUSE_II:
            putstr("Illegal instruction\n");
            break;
        case SCAUSE_B:
            putstr("Breakpoint\n");
            putstr("ebreak caught at 0x"); putstr(uptrtoa(tf->sepc, 16)); putch('\n');
            tf->sepc += 2;
            break;
        case SCAUSE_LAM:
            putstr("Load address misaligned\n");
            break;
        case SCAUSE_LAF:
            putstr("Load access fault\n");
            if ((errno = page_fault_handler(tf))) {
                print_trapframe(tf); 
                putstr("ERRNO: "); putstr(i32toa(errno, 10)); putch('\n');
                PANIC("handle page fault failed.");
            }
            break;
        case SCAUSE_SAM:
            putstr("Store/AMO address misaligned\n");
            break;
        case SCAUSE_SAF:
            putstr("Store/AMO access fault\n");
            if ((errno = page_fault_handler(tf))) {
                print_trapframe(tf); 
                putstr("ERRNO: "); putstr(i32toa(errno, 10)); putch('\n');
                PANIC("handle page fault failed.");
            }
            break;
        case SCAUSE_ECFU:
            putstr("Environment call from U-mode\n");
            break;
        case SCAUSE_ECFS:
            putstr("Environment call from S-mode\n");
            break;
        case SCAUSE_IPF:
            putstr("Instruction page fault\n");
            break;
        case SCAUSE_LPF:
            putstr("Load page fault\n");
            if ((errno = page_fault_handler(tf))) {
                print_trapframe(tf); 
                putstr("ERRNO: "); putstr(i32toa(errno, 10)); putch('\n');
                PANIC("handle page fault failed.");
            }
            break;
        case SCAUSE_SPF:
            putstr("Store/AMO page fault\n");
            if ((errno = page_fault_handler(tf))) {
                print_trapframe(tf); 
                putstr("ERRNO: "); putstr(i32toa(errno, 10)); putch('\n');
                PANIC("handle page fault failed.");
            }
            break;
        default:
            print_trapframe(tf);
            break;
    }
}

void trap(struct trapframe* tf) {
    // The Interrupt bit in the scause is set if the trap was caused by an interrupt
    if (tf->scause & 0x8000000000000000) interrupt_handler(tf);
    else exception_handler(tf);
}

// Load supervisor trap entry in RISC-V
void init_idt() {
    extern void all_traps();
    // Set sscratch register to 0, indicating to exception vector that we are
    // presently executing in the kernel
    CSRW(sscratch, 0);
    // Set the exception vector address
    CSRW(stvec, all_traps);
    intr_enable();
    putstr("init idt\n\n");
}

void print_trapframe(struct trapframe* tf) {
    putstr("trapframe at 0x"); putstr(uptrtoa((uintptr_t) tf, 16)); putch('\n');
    print_registers(&tf->pushregs);
    putstr("\tsstatus\t0x"); putstr(uptrtoa(tf->sstatus, 16)); putch('\n');
    putstr("\tsepc\t0x"); putstr(uptrtoa(tf->sepc, 16)); putch('\n');
    putstr("\tstval\t0x"); putstr(uptrtoa(tf->stval, 16)); putch('\n');
    putstr("\tscause\t0x"); putstr(uptrtoa(tf->scause, 16)); putch('\n');
    putch('\n');
}

void print_registers(struct registers* regs) {
    putstr("\tzero\t0x"); putstr(uptrtoa(regs->zero, 16)); putch('\n');
    putstr("\tra\t0x"); putstr(uptrtoa(regs->ra, 16)); putch('\n');
    putstr("\tsp\t0x"); putstr(uptrtoa(regs->sp, 16)); putch('\n');
    putstr("\tgp\t0x"); putstr(uptrtoa(regs->gp, 16)); putch('\n');
    putstr("\ttp\t0x"); putstr(uptrtoa(regs->tp, 16)); putch('\n');
    putstr("\tt0\t0x"); putstr(uptrtoa(regs->t0, 16)); putch('\n');
    putstr("\tt1\t0x"); putstr(uptrtoa(regs->t1, 16)); putch('\n');
    putstr("\tt2\t0x"); putstr(uptrtoa(regs->t2, 16)); putch('\n');
    putstr("\ts0\t0x"); putstr(uptrtoa(regs->s0, 16)); putch('\n');
    putstr("\ts1\t0x"); putstr(uptrtoa(regs->s1, 16)); putch('\n');
    putstr("\ta0\t0x"); putstr(uptrtoa(regs->a0, 16)); putch('\n');
    putstr("\ta1\t0x"); putstr(uptrtoa(regs->a1, 16)); putch('\n');
    putstr("\ta2\t0x"); putstr(uptrtoa(regs->a2, 16)); putch('\n');
    putstr("\ta3\t0x"); putstr(uptrtoa(regs->a3, 16)); putch('\n');
    putstr("\ta4\t0x"); putstr(uptrtoa(regs->a4, 16)); putch('\n');
    putstr("\ta5\t0x"); putstr(uptrtoa(regs->a5, 16)); putch('\n');
    putstr("\ta6\t0x"); putstr(uptrtoa(regs->a6, 16)); putch('\n');
    putstr("\ta7\t0x"); putstr(uptrtoa(regs->a7, 16)); putch('\n');
    putstr("\ts2\t0x"); putstr(uptrtoa(regs->s2, 16)); putch('\n');
    putstr("\ts3\t0x"); putstr(uptrtoa(regs->s3, 16)); putch('\n');
    putstr("\ts4\t0x"); putstr(uptrtoa(regs->s4, 16)); putch('\n');
    putstr("\ts5\t0x"); putstr(uptrtoa(regs->s5, 16)); putch('\n');
    putstr("\ts6\t0x"); putstr(uptrtoa(regs->s6, 16)); putch('\n');
    putstr("\ts7\t0x"); putstr(uptrtoa(regs->s7, 16)); putch('\n');
    putstr("\ts8\t0x"); putstr(uptrtoa(regs->s8, 16)); putch('\n');
    putstr("\ts9\t0x"); putstr(uptrtoa(regs->s9, 16)); putch('\n');
    putstr("\ts10\t0x"); putstr(uptrtoa(regs->s10, 16)); putch('\n');
    putstr("\ts11\t0x"); putstr(uptrtoa(regs->s11, 16)); putch('\n');
    putstr("\tt3\t0x"); putstr(uptrtoa(regs->t3, 16)); putch('\n');
    putstr("\tt4\t0x"); putstr(uptrtoa(regs->t4, 16)); putch('\n');
    putstr("\tt5\t0x"); putstr(uptrtoa(regs->t5, 16)); putch('\n');
    putstr("\tt6\t0x"); putstr(uptrtoa(regs->t6, 16)); putch('\n');
    putch('\n');
}
