#ifndef __KERNEL_PROC_PROC_H__
#define __KERNEL_PROC_PROC_H__

#include "defs.h"
#include "list.h"
#include "errno.h"
#include "page.h"
#include "riscv.h"
#include "memlayout.h"

// ------------- process/thread mechanism design&implementation -------------
// (an simplified Linux process/thread mechanism )
// introduction:
//   ucore implements a simple process/thread mechanism. process contains the independent memory sapce, at least one threads
// for execution, the kernel data(for management), processor state (for context switch), files(in lab6), etc. ucore needs to
// manage all these details efficiently. In ucore, a thread is just a special kind of process(share process's memory).
// ------------------------------
// process state       :     meaning               -- reason
//     PROC_UNINIT     :   uninitialized           -- alloc_proc
//     PROC_SLEEPING   :   sleeping                -- try_free_pages, do_wait, do_sleep
//     PROC_RUNNABLE   :   runnable(maybe running) -- proc_init, wakeup_proc, 
//     PROC_ZOMBIE     :   almost dead             -- do_exit

// -----------------------------
// process state changing:
                                            
//   alloc_proc                                 RUNNING
//       +                                   +--<----<--+
//       +                                   + proc_run +
//       V                                   +-->---->--+ 
// PROC_UNINIT -- proc_init/wakeup_proc --> PROC_RUNNABLE -- try_free_pages/do_wait/do_sleep --> PROC_SLEEPING --
//                                            A      +                                                           +
//                                            |      +--- do_exit --> PROC_ZOMBIE                                +
//                                            +                                                                  + 
//                                            -----------------------wakeup_proc----------------------------------
// -----------------------------
// process relations
// parent:              proc->parent        (proc is child)
// child:               proc->child         (proc is parent)
// older sibling:       proc->older         (proc is younger sibling)
// younger sibling:     proc->younger       (proc is older sibling)
// -----------------------------
// related syscall for process:
// SYS_exit        : process exit,                           -->do_exit
// SYS_fork        : create child process, dup mm            -->do_fork-->wakeup_proc
// SYS_wait        : wait process                            -->do_wait
// SYS_exec        : after fork, process execute a program   -->load a program and refresh the mm
// SYS_clone       : create child thread                     -->do_fork-->wakeup_proc
// SYS_yield       : process flag itself need resecheduling, -- proc->need_reschedule=1, then scheduler will rescheule this process
// SYS_sleep       : process sleep                           -->do_sleep 
// SYS_kill        : kill process                            -->do_kill-->proc->flags |= PROC_FLAG_EXIT
//                                                                     -->wakeup_proc-->do_wait-->do_exit   
// SYS_getpid      : get the process's pid

#define PROC_NAME_LEN   16
#define MAX_PROCESS     4096
#define MAX_PID         (MAX_PROCESS << 1)

#define PROC_UNINIT     0   // uinitialized
#define PROC_SLEEPING   1   // sleeping
#define PROC_RUNNABLE   2   // runnable/running
#define PROC_ZOMBIE     3   // almost dead, and wait parent proc to reclaim his resource

#define WAIT_NONE       0x00000000                  // not waiting
#define WAIT_CHILD      (0x00000001 | WAIT_INT)
#define WAIT_INT        0x80000000                  // the wait state could be interrupted

#define PROC_FLAG_EXIT  0x00000001                  // getting shutdown

static void write_satp(u64 addr) {
    CSRW(satp, 0x8000000000000000 | (addr >> PAGE_SHIFT));
}


struct context {
    u64 ra;       // Return address
    u64 sp;       // Stack pointer
    u64 s0;       // Saved register/frame pointer
    u64 s1;       // Saved register
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
};

struct pcb {
    struct list list_link;
    struct list hash_link;
    u32 state;
    u32 wait_state;
    i32 pid;                // process ID
    i32 run_times;          // the running times of process
    u64 kernel_stack;
    volatile bool need_reschedule;   // need to be need_rescheduled to release CPU?
    struct vm_manager* vmm;
    struct context context;     // switch here to run process
    struct trapframe* tf;       // trap frame for current interrupt
    pte_t* p_gpt;           // the GPTE of this process
    u32 flags;
    char name[PROC_NAME_LEN];
    i32 exit_code;          // be sent to the parent process

    struct pcb* parent;
    struct pcb* child;
    struct pcb* younger;
    struct pcb* older;
};

#define LIST2PCB(member, list) \
    TO_STRUCT(struct pcb, member, (list))

extern struct list g_proc_list;

extern struct pcb* g_idle_proc;
extern struct pcb* g_curr_proc;

void init_proc();

void run_idle() __attribute__((noreturn));

void run_proc(struct pcb* proc);

// do_wait - wait one OR any child with PROC_ZOMBIE state, and free memory space of 
//         - kernel stack pcb of this child.
// NOTE: only after do_wait function, all resources of the child processes are free
i32 do_wait(i32 pid, i32* exit_code);

// do_exit - called by sys_exit
//   1. call del_vmas & free_gpt & del_vmm to free the almost all memory space of process
//   2. set process state as PROC_ZOMBIE, then call wakeup_proc(parent) to ask parent reclaim itself
//   3. call schedule to switch to other process
i32 do_exit(i32 errno);

// do_fork - parent process for a new child process
// @stack  - the parent's user stack pointer. if stack == 0, It means to fork a kernel thread.
i32 do_fork(u64 stack, struct trapframe* tf, u32 clone_flag);

// do_execve - call del_vmas(vmm) & free_gpt(vmm) to reclaim memory space of g_curr_proc
//           - call load_bin to setup new memory space according binary program
i32 do_execve(const char* name, u64 len, u8* bin, u64 size);

// do_yield - ask the scheduler to need_reschedule
i32 do_yield();

// do_kill - kill process with pid by set this process flags with PROC_FLAG_EXIT
i32 do_kill(i32 pid);

#endif // __KERNEL_PROC_H__
