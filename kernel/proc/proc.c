#include "proc.h"

#include "pmm.h"
#include "string.h"
#include "sched.h"
#include "trap.h"
#include "riscv.h"
#include "stdlib.h"
#include "intr.h"


// the process set's list
struct list g_proc_list;

struct pcb* g_idle_proc;
struct pcb* g_curr_proc;

static struct pcb* g_init_proc;

static uint32_t g_num_proc;

// trap_entry.S
void fork_ret(struct trapframe* tf);

// curr_fork_ret -- the first kernel entry point of a new process
// NOTE: the addr of curr_fork_ret is setted in copy_proc function
//       after switch_to, the current proc will execute here.
static inline void curr_fork_ret() {
    fork_ret(g_curr_proc->tf);
}

// proc_entry.S
void kernel_proc_entry();

// switch.S
void switch_proc(struct context* from, struct context* to);

// alloc_proc - alloc a struct pcb and init all fields of pcb
static struct pcb* alloc_proc() {
    struct pcb* proc = malloc(sizeof(struct pcb));
    if (proc) {
        proc->state = PROC_UNINIT;
        proc->pid = -1;
        proc->run_times = 0;
        proc->kernel_stack = 0;
        proc->reschedule = 0;
        proc->parent = NULL;
        proc->vmm = NULL;
        proc->context = (struct context) { 0 };
        proc->tf = NULL;
        proc->flags = 0;
        memset(proc->name, 0, sizeof(proc->name));
    }
    return proc;
}

// setup_kernel_stack - alloc pages with KERNEL_STACK_PAGE as process kernel stack
static int32_t setup_kernel_stack(struct pcb* proc) {
    struct page* page = alloc_pages(KERNEL_STACK_PAGE);
    if (page) {
        proc->kernel_stack = page2va(page);
        return 0;
    }
    return ENOMEM;
}

// copy_proc - setup the trapframe on the  process's kernel stack top and
//           - setup the kernel entry point and stack of process
static void copy_proc(struct pcb* proc, uintptr_t stack, struct trapframe* tf) {
    proc->tf = (struct trapframe*) 
        (proc->kernel_stack + KERNEL_STACK_SIZE - sizeof(struct trapframe));
    *proc->tf = *tf;
    
    proc->tf->regs.a0 = 0;
    proc->tf->regs.sp = (stack) ? stack : (uintptr_t) proc->tf;

    proc->context.ra = (uintptr_t) curr_fork_ret;
    proc->context.sp = (uintptr_t) proc->tf;
}

static int32_t alloc_pid() {
    static int32_t next_pid = MAX_PID;
    static int32_t prev_pid = MAX_PID;
    prev_pid = prev_pid + 1 >= MAX_PID ? 1 : prev_pid + 1;

    if (prev_pid >= next_pid || prev_pid == 1) {
        next_pid = MAX_PID;
        struct list* list = &g_proc_list;
        while ((list = list->next) != &g_proc_list) {
            struct pcb* proc = LIST2PCB(list_link, list);
            if (proc->pid == prev_pid) {
                if (++prev_pid >= next_pid) {
                    if (prev_pid >= MAX_PID)
                        prev_pid = 1;
                    next_pid = MAX_PID;
                    list = &g_proc_list;
                    continue;
                }
            } else if (prev_pid < proc->pid && proc->pid < next_pid) {
                next_pid = proc->pid;
            }
        }
    }
    return prev_pid;
}

// do_fork - parent process for a new child process
// @stack  - the parent's user stack pointer. if stack == 0, It means to fork a kernel thread.
static int32_t do_fork(uintptr_t stack, struct trapframe* tf) {
    int32_t errno = -ESRCH;
    if (g_num_proc > MAX_PROCESS) 
        return errno;
    errno = -ENOMEM;
    struct pcb* proc = NULL;
    if (!(proc = alloc_proc()))
        return errno;
    if ((setup_kernel_stack(proc)) == ENOMEM) {
        free(proc, sizeof(struct pcb));
        return errno;
    }
    copy_proc(proc, stack, tf);
    proc->pid = alloc_pid();
    add_list(&g_proc_list, g_proc_list.next, &proc->list_link);
    g_num_proc++;
    wakeup_proc(proc);
    return proc->pid;
}

// kernel_thread - create a kernel proc using "func" function
// NOTE: the contents of temp trapframe tf will be copied to 
//       proc->tf in do_fork-->copy_proc function
static int32_t kernel_proc(int32_t (*func) (void *), void* arg) {
    struct trapframe tf = { 0 };

    tf.regs.s0 = (uintptr_t) func;
    tf.regs.s1 = (uintptr_t) arg;
    tf.sstatus = (CSRR(sstatus) | SSTATUS_SPIE | SSTATUS_SPP) & ~SSTATUS_SIE;
    tf.sepc = (uintptr_t) kernel_proc_entry;
    return do_fork(0, &tf);
}

static int32_t init_main(void* arg) {
    putstr("this init process, pid = "); putstr(i32toa(g_curr_proc->pid, 10)); 
    putstr(", name = "); putstr(g_curr_proc->name); putch('\n');
    putstr("arg: "); putstr((const char*) arg); putch('\n');
    return 0;
}

static struct pcb* find_proc(int32_t pid) {
    if (pid > 0 && pid < MAX_PID) {
        struct list* list = &g_proc_list;
        while ((list = list->next) != &g_proc_list) {
            struct pcb* proc = LIST2PCB(list_link, list);
            if (proc->pid == pid)
                return proc;
        }
    }
    return NULL;
}

void do_exit(int32_t errno) {
    putstr("process exit!\n\n");
    while (1)
        continue;
}

void init_proc() {
    init_list(&g_proc_list);
    
    ASSERT(g_idle_proc = alloc_proc());

    g_idle_proc->pid = 0;
    g_idle_proc->state = PROC_RUNNABLE;
    extern byte boot_stack[];
    g_idle_proc->kernel_stack = (uintptr_t) boot_stack;
    g_idle_proc->reschedule = 1;
    strcpy(g_idle_proc->name, "idle");

    g_num_proc++;
    g_curr_proc = g_idle_proc;
    add_list(&g_proc_list, g_proc_list.next, &g_idle_proc->list_link);

    int32_t pid = kernel_proc(init_main, "Garret-OS - init_main arg");
    if (pid <= 0) 
        PANIC("create init_main failed.");
    
    g_init_proc = find_proc(pid);
    strcpy(g_init_proc->name, "init");

    ASSERT(g_idle_proc != NULL && g_idle_proc->pid == 0);
    ASSERT(g_curr_proc != NULL && g_curr_proc->pid == 0);
    ASSERT(g_init_proc != NULL && g_init_proc->pid == 1);
    putstr("init process!\n\n");
}

// proc_run - make process "proc" running on cpu
// NOTE: before call switch, should load  base addr of "proc"'s new PDT
void run_proc(struct pcb* proc) {
    if (proc != g_curr_proc) {
        struct pcb* prev = g_curr_proc;
        g_curr_proc = proc;
        putstr("switch from process: "); putstr(prev->name);
        putstr(" -> to process: "); putstr(g_curr_proc->name);
        putstr("\n\n");
        switch_proc(&prev->context, &g_curr_proc->context);
    }
}

void run_idle() {
    while (1)
        if (g_curr_proc->reschedule)
            schedule();
}
