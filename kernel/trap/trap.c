#include "trap.h"

#include "assert.h"
#include "defs.h"
#include "riscv.h"
#include "stdio.h"
#include "stdlib.h"
#include "timer.h"
#include "vmm.h"
#include "intr.h"
#include "syscall.h"

#define TICKS_NUM 100


static inline void print_ticks() {
    putstr(u8toa(TICKS_NUM, 10)); putstr(" ticks\n");
}

static inline bool check_kernel_trap(struct trapframe* tf) {
    return (tf->sstatus & SSTATUS_SPP);
}

static inline void print_page_fault(struct trapframe* tf) {
    putstr("page fault at 0x"); putstr(u64toa(tf->stval, 16));
    putch(check_kernel_trap(tf) ? 'K' : 'U'); putch('/');
    putch((tf->scause == SCAUSE_SPF) ? 'W' : 'R'); putch('\n');
}

static i32 page_fault_handler(struct trapframe* tf) {
    putstr("paged fault at 0x"); putstr(u64toa(tf->stval, 16)); putch('\n');
    if (g_test_vmm)
        print_page_fault(tf);

    struct vm_manager* vmm;
    if (g_test_vmm) {
        ASSERT(g_curr_proc == g_idle_proc);
        vmm = g_test_vmm;
    } else {
        if (!g_curr_proc) {
            print_trapframe(tf);
            print_page_fault(tf);
            PANIC("unhandled page fault.\n");
        }
        vmm = g_curr_proc->vmm;
    }

    return handle_page_fault(vmm, tf->stval);
}

static void interrupt_handler(struct trapframe* tf) {
    switch (tf->scause) {
        case SCAUSE_SSI:
            putstr("Supervisor software interrupt\n");
            break;
        case SCAUSE_STI:
            // putstr("Supervisor timer interrupt\n");
            timer_next();
            if ((!(++g_ticks % TICKS_NUM)) && g_curr_proc)
                g_curr_proc->need_reschedule = true;
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
    i32 errno;
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
            putstr("ebreak caught at 0x"); putstr(u64toa(tf->sepc, 16));
            putstr("\n\n");
            tf->sepc += 2;
            if (tf->gpr.a7 == EBREAK_MAGIC) {
                syscall();
            }
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
            // putstr("Environment call from U-mode\n");
            tf->sepc += 4;
            syscall();
            break;
        case SCAUSE_ECFS:
            putstr("Environment call from S-mode\n");
            tf->sepc += 4;
            syscall();
            break;
        case SCAUSE_IPF:
            putstr("Instruction page fault\n");
            if ((errno = page_fault_handler(tf))) {
                print_trapframe(tf); 
                putstr("ERRNO: "); putstr(i32toa(errno, 10)); putch('\n');
                PANIC("handle page fault failed.");
            }
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

static inline void trap_dispatch(struct trapframe* tf) {
    // The Interrupt bit in the scause is set if the trap was caused by an interrupt
    if (tf->scause & 0x8000000000000000) interrupt_handler(tf);
    else exception_handler(tf);
}

// trap - handles or dispatches an exception/interrupt. if and when trap() returns,
//        the code in kernal/trap/trap_entry.S restores the old CPU state saved in the
//        trapframe and then uses th iret instruction to return from the exception.
void trap(struct trapframe* tf) {
    // dispatch based on what type of trap occured
    if (g_curr_proc) {
        struct trapframe* old_tf = g_curr_proc->tf;
        g_curr_proc->tf = tf;

        bool is_kernel_trap = check_kernel_trap(tf);

        trap_dispatch(tf);

        g_curr_proc->tf = old_tf;
        if (!is_kernel_trap) {
            if (g_curr_proc->flags & PROC_FLAG_EXIT)
                do_exit(-E_KILLED);
            if (g_curr_proc->need_reschedule)
                schedule();
        }
    } else {
        trap_dispatch(tf);
    }
}

// Load supervisor trap entry in RISC-V
void init_idt() {
    extern void all_traps();
    // Set sscratch register to 0, indicating to exception vector that we are
    // presently executing in the kernel
    CSRW(sscratch, 0);
    // Set the exception vector address
    CSRW(stvec, all_traps);
    // Allow kernel to access user memory
    // 实验临时做法，正常操作系统应当实现copy_to_user函数而不应该开启SSTATUS_SUM
    CSRRS(sstatus, SSTATUS_SUM);

    intr_enable();
    putstr("init idt\n\n");
}

void print_trapframe(struct trapframe* tf) {
    putstr("trapframe at 0x"); putstr(u64toa((u64) tf, 16)); putch('\n');
    print_registers(&tf->gpr);
    putstr("\tsstatus\t0x"); putstr(u64toa(tf->sstatus, 16)); putch('\n');
    putstr("\tsepc\t0x"); putstr(u64toa(tf->sepc, 16)); putch('\n');
    putstr("\tstval\t0x"); putstr(u64toa(tf->stval, 16)); putch('\n');
    putstr("\tscause\t0x"); putstr(u64toa(tf->scause, 16)); putch('\n');
    putch('\n');
}

void print_registers(struct gpr* p_gpr) {
    putstr("\tzero\t0x"); putstr(u64toa(p_gpr->zero, 16)); putch('\n');
    putstr("\tra\t0x"); putstr(u64toa(p_gpr->ra, 16)); putch('\n');
    putstr("\tsp\t0x"); putstr(u64toa(p_gpr->sp, 16)); putch('\n');
    putstr("\tgp\t0x"); putstr(u64toa(p_gpr->gp, 16)); putch('\n');
    putstr("\ttp\t0x"); putstr(u64toa(p_gpr->tp, 16)); putch('\n');
    putstr("\tt0\t0x"); putstr(u64toa(p_gpr->t0, 16)); putch('\n');
    putstr("\tt1\t0x"); putstr(u64toa(p_gpr->t1, 16)); putch('\n');
    putstr("\tt2\t0x"); putstr(u64toa(p_gpr->t2, 16)); putch('\n');
    putstr("\ts0\t0x"); putstr(u64toa(p_gpr->s0, 16)); putch('\n');
    putstr("\ts1\t0x"); putstr(u64toa(p_gpr->s1, 16)); putch('\n');
    putstr("\ta0\t0x"); putstr(u64toa(p_gpr->a0, 16)); putch('\n');
    putstr("\ta1\t0x"); putstr(u64toa(p_gpr->a1, 16)); putch('\n');
    putstr("\ta2\t0x"); putstr(u64toa(p_gpr->a2, 16)); putch('\n');
    putstr("\ta3\t0x"); putstr(u64toa(p_gpr->a3, 16)); putch('\n');
    putstr("\ta4\t0x"); putstr(u64toa(p_gpr->a4, 16)); putch('\n');
    putstr("\ta5\t0x"); putstr(u64toa(p_gpr->a5, 16)); putch('\n');
    putstr("\ta6\t0x"); putstr(u64toa(p_gpr->a6, 16)); putch('\n');
    putstr("\ta7\t0x"); putstr(u64toa(p_gpr->a7, 16)); putch('\n');
    putstr("\ts2\t0x"); putstr(u64toa(p_gpr->s2, 16)); putch('\n');
    putstr("\ts3\t0x"); putstr(u64toa(p_gpr->s3, 16)); putch('\n');
    putstr("\ts4\t0x"); putstr(u64toa(p_gpr->s4, 16)); putch('\n');
    putstr("\ts5\t0x"); putstr(u64toa(p_gpr->s5, 16)); putch('\n');
    putstr("\ts6\t0x"); putstr(u64toa(p_gpr->s6, 16)); putch('\n');
    putstr("\ts7\t0x"); putstr(u64toa(p_gpr->s7, 16)); putch('\n');
    putstr("\ts8\t0x"); putstr(u64toa(p_gpr->s8, 16)); putch('\n');
    putstr("\ts9\t0x"); putstr(u64toa(p_gpr->s9, 16)); putch('\n');
    putstr("\ts10\t0x"); putstr(u64toa(p_gpr->s10, 16)); putch('\n');
    putstr("\ts11\t0x"); putstr(u64toa(p_gpr->s11, 16)); putch('\n');
    putstr("\tt3\t0x"); putstr(u64toa(p_gpr->t3, 16)); putch('\n');
    putstr("\tt4\t0x"); putstr(u64toa(p_gpr->t4, 16)); putch('\n');
    putstr("\tt5\t0x"); putstr(u64toa(p_gpr->t5, 16)); putch('\n');
    putstr("\tt6\t0x"); putstr(u64toa(p_gpr->t6, 16)); putch('\n');
    putch('\n');
}
