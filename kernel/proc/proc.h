#ifndef __KERNEL_PROC_PROC_H__
#define __KERNEL_PROC_PROC_H__

#include "defs.h"
#include "list.h"
#include "errno.h"

#define PROC_NAME_LEN   16
#define MAX_PROCESS     4096
#define MAX_PID         (MAX_PROCESS << 1)

#define PROC_UNINIT     0   // uinitialized
#define PROC_SLEEPING   1   // sleeping
#define PROC_RUNNABLE   2   // runnable/running
#define PROC_ZOMBIE     3   // almost dead, and wait parent proc to reclaim his resource


struct context {
    uintptr_t ra;       // Return address
    uintptr_t sp;       // Stack pointer
    uintptr_t s0;       // Saved register/frame pointer
    uintptr_t s1;       // Saved register
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
};

struct pcb {
    struct list list_link;
    uint32_t state;
    int32_t pid;                // process ID
    int32_t run_times;          // the running times of process
    uintptr_t kernel_stack;
    volatile bool reschedule;   // need to be rescheduled to release CPU?
    struct process* parent;
    struct vm_manager* vmm;
    struct context context;     // switch here to run process
    struct trapframe* tf;       // trap frame for current interrupt
    uint32_t flags;
    char name[PROC_NAME_LEN];
};

#define LIST2PCB(member, list) \
    TO_STRUCT(struct pcb, member, (list))

extern struct list g_proc_list;

extern struct pcb* g_idle_proc;
extern struct pcb* g_curr_proc;

void init_proc();

void run_idle() __attribute__((noreturn));

void run_proc(struct pcb* proc);

void do_exit(int32_t errno);

#endif // __KERNEL_PROC_H__
