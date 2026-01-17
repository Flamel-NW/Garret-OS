#include "proc.h"

#include "pmm.h"
#include "string.h"
#include "schedule.h"
#include "trap.h"
#include "riscv.h"
#include "stdlib.h"
#include "intr.h"
#include "unistd.h"
#include "vmm.h"
#include "elf.h"


#define HASH_SHIFT      10
#define HASH_LIST_SIZE  (1 << 10)
#define PID_HASH(x)     (hash32(x, HASH_SHIFT))

// hash list for process set based on pid
static struct list g_hash_list[HASH_LIST_SIZE];

// the process set's list
struct list g_proc_list;

struct pcb* g_idle_proc;
struct pcb* g_curr_proc;

static struct pcb* g_init_proc;

static u32 g_num_proc;

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

static u64 kernel_execve(const char* name, u8* binary, u64 size) {
    putstr("kernel_execve: pid = "); putstr(i32toa(g_curr_proc->pid, 10));
    putstr(", name = \""); putstr(name); putstr("\"\n");

    u64 ret = 0;
    u64 len = strlen(name);
    __asm__ volatile (
        "li a0, %1\n"
        "ld a1, %2\n"
        "ld a2, %3\n"
        "ld a3, %4\n"
        "ld a4, %5\n"
        "li a7, %6\n"
        "ebreak\n"
        "sd a0, %0\n"
        : "=m" (ret)
        : "i" (SYS_EXEC), "m" (name), "m" (len), "m" (binary), "m" (size), "i" (EBREAK_MAGIC)
        : "memory"
    );
    putstr("ret = "); putstr(u64toa(ret, 10)); putch('\n');
    return ret;
}

#define KERNEL_EXECVE_TEST(x) KERNEL_EXECVE(x)

#define KERNEL_EXECVE(x) ({                                         \
    extern u8 _binary_user_##x##_out_start[],                       \
        _binary_user_##x##_out_size[];                              \
    kernel_execve(#x, _binary_user_##x##_out_start,                 \
        (u64) _binary_user_##x##_out_size);                         \
})

// set_links - set the relation links of proc
static void set_links(struct pcb* proc) {
    add_list(&g_proc_list, g_proc_list.next, &proc->list_link);
    proc->younger = NULL;
    if ((proc->older = proc->parent->older) != NULL) 
        proc->older->younger = proc;
    proc->parent->child = proc;
    g_num_proc++;
}

// remove_links - clean the relation links of proc
static void remove_links(struct pcb* proc) {
    del_list(&proc->list_link);

    if (proc->older != NULL)
        proc->older->younger = proc->younger;
    if (proc->younger != NULL) 
        proc->younger->older = proc->older;
    else
        proc->parent->child = proc->older;

    g_num_proc--;
}

// hash_proc - add proc into proc hash_list
static void hash_proc(struct pcb* proc) {
    add_list(&g_hash_list[PID_HASH(proc->pid)], g_hash_list[PID_HASH(proc->pid)].next, &proc->hash_link);
}

// unhash_proc - delete proc from proc hash_list
static void unhash_proc(struct pcb* proc) {
    del_list(&proc->hash_link);
}

// alloc_proc - alloc a struct pcb and init all fields of pcb
static struct pcb* alloc_proc() {
    struct pcb* proc = malloc(sizeof(struct pcb));
    if (proc) {
        proc->state = PROC_UNINIT;
        proc->pid = -1;
        proc->run_times = 0;
        proc->kernel_stack = 0;
        proc->need_reschedule = false;
        proc->parent = NULL;
        proc->vmm = NULL;
        proc->context = (struct context) { 0 };
        proc->tf = NULL;
        proc->p_gpt = g_bpt;  // Initialize to kernel boot page table (like ucore's boot_cr3)
        proc->flags = 0;
        memset(proc->name, 0, sizeof(proc->name));
    }
    return proc;
}

// alloc_kernel_stack - alloc pages with KERNEL_STACK_PAGE as process kernel stack
static i32 alloc_kernel_stack(struct pcb* proc) {
    struct page* page = alloc_pages(KERNEL_STACK_PAGE);
    if (page) {
        proc->kernel_stack = page2va(page);
        return 0;
    }
    return -E_NO_MEM;
}

// alloc_gpte - alloc one page as page tgable
static i32 alloc_gpt(struct vm_manager* vmm) {
    struct page* page;
    if (!(page = alloc_page())) 
        return -E_NO_MEM;

    pte_t* p_gpt = (pte_t*) page2va(page);
    memcpy(p_gpt, g_bpt, PAGE_SIZE);

    vmm->p_gpt = p_gpt;
    return 0;
}

// free_gpt - free the memory space of page table
static void free_gpt(struct vm_manager* vmm) {
    free_page(va2page((u64) vmm->p_gpt));
}

// free_kernel_stack - free the memory space of process kernel stack
static void free_kernel_stack(struct pcb* proc) {
    free_pages(va2page(proc->kernel_stack), KERNEL_STACK_PAGE);
}

// copy_vmm - process "proc" duplicate OR share process "current"'s vmm according clone_flag
//            if clone_flag & CLONE_VM, then "share"; else "duplicate"
static i32 copy_vmm(struct pcb* proc, u32 clone_flag) {
    struct vm_manager* vmm;
    struct vm_manager* old_vmm = g_curr_proc->vmm;

    // current is a kernel proc
    if (!old_vmm)
        return 0;

    if (clone_flag & CLONE_VM) {
        vmm = old_vmm;
        vmm->use_count++;
        proc->vmm = vmm;
        proc->p_gpt = vmm->p_gpt;
        return 0;
    }

    i32 ret = -E_NO_MEM;
    if (!(vmm = init_vmm())) 
        return ret;
    
    if (alloc_gpt(vmm)) {
        del_vmm(vmm);
        return ret;
    }

    lock(&(old_vmm->lock));
    {
        ret = dup_vmm(old_vmm, vmm);
    }
    unlock(&(old_vmm->lock));

    if (ret) {
        del_vmas(vmm);
        free_gpt(vmm);
        del_vmm(vmm);
        return ret;
    }

    vmm->use_count++;
    proc->vmm = vmm;
    proc->p_gpt = vmm->p_gpt;
    
    return 0;
}

// copy_proc - setup the trapframe on the  process's kernel stack top and
//           - setup the kernel entry point and stack of process
static void copy_proc(struct pcb* proc, u64 stack, struct trapframe* tf) {
    proc->tf = (struct trapframe*) 
        (proc->kernel_stack + KERNEL_STACK_SIZE - sizeof(struct trapframe));
    *(proc->tf) = *tf;
    
    proc->tf->gpr.a0 = 0;
    proc->tf->gpr.sp = stack ? stack : ((u64) proc->tf - 4);

    proc->context.ra = (u64) curr_fork_ret;
    proc->context.sp = (u64) proc->tf;
}

static i32 alloc_pid() {
    static i32 next_pid = MAX_PID;
    static i32 prev_pid = MAX_PID;
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
i32 do_fork(u64 stack, struct trapframe* tf, u32 clone_flag) {
    i32 ret = -E_NO_FREE_PROC;
    if (g_num_proc > MAX_PROCESS) 
        return ret;
    ret = -E_NO_MEM;

    struct pcb* proc = NULL;
    if (!(proc = alloc_proc()))
        return ret;

    proc->parent = g_curr_proc;
    ASSERT(g_curr_proc->wait_state == WAIT_NONE);

    if ((alloc_kernel_stack(proc)) == -E_NO_MEM) {
        free(proc, sizeof(struct pcb));
        return ret;
    }
    if ((copy_vmm(proc, clone_flag)) == -E_NO_MEM) {
        free_kernel_stack(proc);
        free(proc, sizeof(struct pcb));
        return ret;
    }
    copy_proc(proc, stack, tf);

    bool intr_flag = local_intr_save();
    {
        proc->pid = alloc_pid();
        hash_proc(proc);
        set_links(proc);
    }
    local_intr_restore(intr_flag);

    wakeup_proc(proc);
    return proc->pid;
}

// kernel_proc - create a kernel proc using "func" function
// NOTE: the contents of temp trapframe tf will be copied to 
//       proc->tf in do_fork-->copy_proc function
static i32 kernel_proc(i32 (*func) (void *), void* arg, u32 clone_flag) {
    struct trapframe tf = { 0 };
    tf.gpr.s0 = (u64) func;
    tf.gpr.s1 = (u64) arg;
    tf.sstatus = (CSRR(sstatus) | SSTATUS_SPIE | SSTATUS_SPP) & ~SSTATUS_SIE;
    tf.sepc = (u64) kernel_proc_entry;
    return do_fork(0, &tf, clone_flag | CLONE_VM);
}

static i32 user_main(void* arg) {
#ifdef TEST
    KERNEL_EXECVE_TEST(TEST);
#else
    KERNEL_EXECVE(exit);
#endif
    PANIC("user_main execve failed!");
}

static i32 init_main(void* arg) {
    putstr("this init process, pid = "); putstr(i32toa(g_curr_proc->pid, 10)); 
    putstr(", name = "); putstr(g_curr_proc->name); putch('\n');
    putstr("arg: "); putstr((const char*) arg); putch('\n');

    u64 num_free_pages = count_free_pages();

    i32 pid = kernel_proc(user_main, NULL, 0);

    if (pid <= 0) 
        PANIC("create user_name failed");
    
    // Set process name for the newly created child process
    struct pcb* proc = g_curr_proc->child;
    if (proc && proc->pid == pid)
        strcpy(proc->name, "user_main");

    while (do_wait(0, NULL) == 0)
        schedule();

    putstr("all user-mode processes have quit.\n");
    ASSERT(g_init_proc->child == NULL && g_init_proc->younger == NULL && g_init_proc->older == NULL);
    ASSERT(g_num_proc == 2);
    ASSERT(g_proc_list.next == &(g_init_proc->list_link));
    ASSERT(g_proc_list.prev == &(g_init_proc->list_link));
    putstr("init check memory pass.\n");

    return 0;
}

static struct pcb* get_proc(i32 pid) {
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

// do_wait - wait one OR any child with PROC_ZOMBIE state, and free memory space of 
//         - kernel stack pcb of this child.
// NOTE: only after do_wait function, all resources of the child processes are free
i32 do_wait(i32 pid, i32* exit_code) {
    struct vm_manager* vmm = g_curr_proc->vmm;
    if (exit_code && !check_vmm(vmm, (u64)exit_code, sizeof(i32), 1))
        return -E_INVAL;

    struct pcb* proc;

    bool has_child = false;
    while (true) {
        if (pid != 0) {
            proc = get_proc(pid);
            if (proc != NULL && proc->parent == g_curr_proc) {
                has_child = true;
                if (proc->state == PROC_ZOMBIE)
                    break;
            }
        } else {
            proc = g_curr_proc->child;
            for ( ; proc; proc = proc->older) {
                has_child = true;
                if (proc->state == PROC_ZOMBIE) 
                    break;
            }
        }

        if (has_child) {
            g_curr_proc->state = PROC_SLEEPING;
            g_curr_proc->wait_state = WAIT_CHILD;
            schedule();
            if (g_curr_proc->flags & PROC_FLAG_EXIT) 
                do_exit(-E_KILLED);
            continue;
        }

        return -E_BAD_PROC;
    }

    if (proc == g_idle_proc || proc == g_init_proc)
        PANIC("wait idle proc or init proc");

    if (exit_code) 
        *exit_code = proc->exit_code;

    bool intr_flag = local_intr_save();
    {
        unhash_proc(proc);
        remove_links(proc);
    }
    local_intr_restore(intr_flag);
        
    free_kernel_stack(proc);
    free(proc, sizeof(struct pcb));

    return 0;
}

// do_exit - called by sys_exit
//   1. call del_vmas & free_gpt & del_vmm to free the almost all memory space of process
//   2. set process state as PROC_ZOMBIE, then call wakeup_proc(parent) to ask parent reclaim itself
//   3. call schedule to switch to other process
i32 do_exit(i32 errno) {
    if (g_curr_proc == g_idle_proc)
        PANIC("idle proc can not exit");
    if (g_curr_proc == g_init_proc)
        PANIC("init proc can not exit");

    struct vm_manager* vmm = g_curr_proc->vmm;
    if (vmm) {
        write_satp(va2pa(((u64) g_bpt)));
        flush_tlb();
        vmm->use_count--;
        if (!vmm->use_count) {
            del_vmas(vmm);
            free_gpt(vmm);
            del_vmm(vmm);
        }
        g_curr_proc->vmm = NULL;
    }

    g_curr_proc->state = PROC_ZOMBIE;
    g_curr_proc->exit_code = errno;

    struct pcb* proc;
    bool intr_flag = local_intr_save();
    {
        proc = g_curr_proc->parent;
        if (proc->wait_state == WAIT_CHILD)
            wakeup_proc(proc);
        while (g_curr_proc->child) {
            proc = g_curr_proc->child;
            g_curr_proc->child = proc->older;
            proc->younger = NULL;

            if ((proc->older = g_init_proc->child))
                g_init_proc->child->younger = proc;

            proc->parent = g_init_proc;
            g_init_proc->child = proc;

            if (proc->state == PROC_ZOMBIE)
                if (g_init_proc->wait_state == WAIT_CHILD)
                    wakeup_proc(g_init_proc);
        }
    }
    local_intr_restore(intr_flag);
    schedule();
    putstr("pid = "); putstr(i32toa(g_curr_proc->pid, 10));
    PANIC(" -- do_exit will not return!!");
    return 0;
}

// load_bin - load the content of binary procgram(ELF format) as the new content of current process
// @bin: the memory addr of the content of binary program
// @size: the size of the content of binary program
static i32 load_bin(u8* bin, u64 size) {
    i32 ret = 0;
    if (g_curr_proc->vmm)
        PANIC("load_bin: g_curr_proc->vmm must be empty.");

    struct vm_manager* vmm;

    // create a new vmm for current process
    if (!(vmm = init_vmm()))
        return -E_NO_MEM;

    // create a new gpt, and vmm->p_gpt = kernel virtual addr of gpt
    if (alloc_gpt(vmm)) {
        del_vmm(vmm);
        return -E_NO_MEM;
    }

    // copy TEST/DATA section, build BSS parts in binary to memory space of process
    struct page* page;
    // get the file header of the binary program (ELF format)
    struct elf_header* elf = (struct elf_header*) bin;
    // This program is valid?
    if (elf->e_magic != ELF_MAGIC) {
        free_gpt(vmm);
        del_vmm(vmm);
        return -E_INVAL_ELF;
    }

    // get the entry of the program section headers of the binary program (ELF format)
    u32 vm_flags = 0;
    struct prog_header* prog = (struct prog_header*) (bin + elf->e_phoff);
    struct prog_header* prog_end = prog + elf->e_phnum;
    for ( ; prog < prog_end; prog++) {
        // find every program section headers
        if (prog->p_type != ELF_PT_LOAD)
            continue;

        if (prog->p_file_size > prog->p_mem_size) {
            del_vmas(vmm);
            free_gpt(vmm);
            del_vmm(vmm);
            return -E_INVAL_ELF;
        }

        // call vm_map fun to setup the new vma (prog->p_va, ph->p_mem_size)
        u32 pte_flags = PTE_U | PTE_V;
        if (prog->p_flags & ELF_PF_X) vm_flags |= VMA_EXEC;
        if (prog->p_flags & ELF_PF_W) vm_flags |= VMA_WRITE;
        if (prog->p_flags & ELF_PF_R) vm_flags |= VMA_READ;
        // modify the perm bits here for RISC-V
        if (vm_flags & VMA_READ) pte_flags |= PTE_R;
        if (vm_flags & VMA_WRITE) pte_flags |= (PTE_W | PTE_R);
        if (vm_flags & VMA_EXEC) pte_flags |= PTE_X;

        if ((ret = vm_map(vmm, prog->p_va, prog->p_mem_size, vm_flags, NULL))) {
            del_vmas(vmm);
            free_gpt(vmm);
            del_vmm(vmm);
            return ret;
        }

        u64 from_begin = (u64) bin + prog->p_offset;
        u64 to_begin = prog->p_va;
        u64 to_end = prog->p_va + prog->p_file_size;
        u64 p_va = rounddown(to_begin, PAGE_SIZE);

        // alloc memory, and copy the contents of every program section (from_begin, from_end) to process memory (to_start, to_end)
        // copy TEXT/DATA section of binary program
        while (to_begin < to_end) {
            if (!(page = alloc_page_gpte(vmm->p_gpt, p_va, pte_flags))) {
                del_vmas(vmm);
                free_gpt(vmm);
                del_vmm(vmm);
                return -E_NO_MEM;
            }
            u64 p_off = to_begin - p_va;
            u64 size = PAGE_SIZE - p_off;
            p_va += PAGE_SIZE;
            if (to_end < p_va)
                size -= p_va - to_end;
            memcpy((void*) (page2va(page) + p_off), (void*) from_begin, size);
            to_begin += size;
            from_begin += size;
        }
        
        // build BSS section of binary program
        to_end = prog->p_va + prog->p_mem_size;
        if (to_begin < p_va) {
            // prog->p_mem_size == prog->p_file_size
            if (to_begin == to_end)
                continue;
            u64 p_off = to_begin + PAGE_SIZE - p_va;
            u64 size = PAGE_SIZE - p_off;
            if (to_end < p_va)
                size -= p_va - to_end;
            memset((void*) (page2va(page) + p_off), 0, size);
            to_begin += size;
            ASSERT((to_end < p_va && to_begin == to_end) || (to_end >= p_va && to_begin == p_va));
        }
        while (to_begin < to_end) {
            if (!(page = alloc_page_gpte(vmm->p_gpt, p_va, pte_flags))) {
                del_vmas(vmm);
                free_gpt(vmm);
                del_vmm(vmm);
                return -E_NO_MEM;
            }
            u64 p_off = to_begin - p_va;
            u64 size = PAGE_SIZE - p_off;
            p_va += PAGE_SIZE;
            if (to_end < p_va)
                size -= p_va - to_end;
            memset((void*) (page2va(page) + p_off), 0, size);
            to_begin += size;
        }
    }
    
    // build user stack memory
    vm_flags = VMA_READ | VMA_WRITE | VMA_STACK;
    if ((ret = vm_map(vmm, USER_STACK_BEGIN, USER_STACK_SIZE, vm_flags, NULL))) {
        del_vmas(vmm);
        free_gpt(vmm);
        del_vmm(vmm);
        return -E_NO_MEM;
    }

    ASSERT(alloc_page_gpte(vmm->p_gpt, USER_STACK_END - PAGE_SIZE, PTE_USER) != NULL);
    ASSERT(alloc_page_gpte(vmm->p_gpt, USER_STACK_END - 2 * PAGE_SIZE, PTE_USER) != NULL);
    ASSERT(alloc_page_gpte(vmm->p_gpt, USER_STACK_END - 3 * PAGE_SIZE, PTE_USER) != NULL);
    ASSERT(alloc_page_gpte(vmm->p_gpt, USER_STACK_END - 4 * PAGE_SIZE, PTE_USER) != NULL);

    // set current process's vmm, satp, and set satp reg = physical addr of Page Table
    vmm->use_count++;
    g_curr_proc->vmm = vmm;
    g_curr_proc->p_gpt = vmm->p_gpt;  // Store kernel virtual address, not physical address
    write_satp(va2pa((u64) vmm->p_gpt));
    flush_tlb(); 

    // setup trapframe for user environment
    struct trapframe* tf = g_curr_proc->tf;
    // keep sstatus;
    u64 sstatus = tf->sstatus;
    memset(tf, 0, sizeof(struct trapframe));

    tf->gpr.sp = USER_STACK_END;
    tf->sepc = elf->e_entry;
    tf->sstatus = sstatus & ~(SSTATUS_SPP | SSTATUS_SPIE);

    return 0;
}

// do_execve - call del_vmas(vmm) & free_gpt(vmm) to reclaim memory space of g_curr_proc
//           - call load_bin to setup new memory space according binary program
i32 do_execve(const char* name, u64 len, u8* bin, u64 size) {
    struct vm_manager* vmm = g_curr_proc->vmm;
    if (!check_vmm(vmm, (u64) name, len, 0))
        return -E_INVAL;
    len = PROC_NAME_LEN ? (len > PROC_NAME_LEN) : len;

    char temp_name[PROC_NAME_LEN + 1];
    strcpy(temp_name, name);

    if (vmm) {
        putstr("vmm != NULL\n");
        write_satp(va2pa((u64) g_bpt));
        flush_tlb();
        vmm->use_count--;
        if (!vmm->use_count) {
            del_vmas(vmm);
            free_gpt(vmm);
            del_vmm(vmm);
        }
        g_curr_proc->vmm = NULL;
    }

    i32 ret;
    if ((ret = load_bin(bin, size))) {
        do_exit(ret);
        putstr("ret = "); putstr(i32toa(ret, 10));
        PANIC(", already exit.\n");
    }
    strcpy(g_curr_proc->name, temp_name);
    return 0;
}

// do_yield - ask the scheduler to need_reschedule
i32 do_yield() {
    g_curr_proc->need_reschedule = true;
    return 0;
}

// do_kill - kill process with pid by set this process flags with PROC_FLAG_EXIT
i32 do_kill(i32 pid) {
    struct pcb* proc;
    if ((proc = get_proc(pid))) {
        if (!(proc->flags & PROC_FLAG_EXIT)) {
            proc->flags |= PROC_FLAG_EXIT;
            if (proc->wait_state & WAIT_INT)
                wakeup_proc(proc);
            return 0;
        }
        return -E_KILLED;
    }
    return -E_INVAL;
}

void init_proc() {
    init_list(&g_proc_list);
    for (i32 i = 0; i < HASH_LIST_SIZE; i++)
        init_list(&g_hash_list[i]);
    
    ASSERT(g_idle_proc = alloc_proc());

    g_idle_proc->pid = 0;
    g_idle_proc->state = PROC_RUNNABLE;
    extern u8 boot_stack[];
    g_idle_proc->kernel_stack = (u64) boot_stack;
    g_idle_proc->need_reschedule = true;
    strcpy(g_idle_proc->name, "idle");

    g_num_proc++;
    g_curr_proc = g_idle_proc;
    add_list(&g_proc_list, g_proc_list.next, &g_idle_proc->list_link);

    i32 pid = kernel_proc(init_main, "Garret-OS - init_main arg", 0);
    if (pid <= 0) 
        PANIC("create init_main failed.");
    
    g_init_proc = get_proc(pid);
    strcpy(g_init_proc->name, "init");

    ASSERT(g_idle_proc != NULL && g_idle_proc->pid == 0);
    ASSERT(g_curr_proc != NULL && g_curr_proc->pid == 0);
    ASSERT(g_init_proc != NULL && g_init_proc->pid == 1);
    putstr("init process!\n\n");
}

// run_proc - make process "proc" running on cpu
// NOTE: before call switch, should load base addr of "proc"'s new PDT
void run_proc(struct pcb* proc) {
    if (proc != g_curr_proc) {
        struct pcb* prev = g_curr_proc;
        g_curr_proc = proc;
        // putstr("switch from process: "); putstr(prev->name);
        // putstr(" -> to process: "); putstr(g_curr_proc->name);
        // putstr("\n\n");
        bool intr_flag = local_intr_save();
        {
            g_curr_proc = proc;
            write_satp(va2pa((u64) proc->p_gpt));
            flush_tlb();
            switch_proc(&prev->context, &g_curr_proc->context);
        }
        local_intr_restore(intr_flag);
    }
}

void run_idle() {
    while (true)
        if (g_curr_proc->need_reschedule)
            schedule();
}
