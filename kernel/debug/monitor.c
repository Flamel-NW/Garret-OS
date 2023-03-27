#include "monitor.h"

#include "debug.h"
#include "defs.h"
#include "stdio.h"
#include "string.h"
#include "trap.h"

static int32_t cmd_help(struct trapframe* tf);
static int32_t cmd_kernel_info(struct trapframe* tf);
static int32_t cmd_backtrace(struct trapframe* tf);

// Simple command-line kernel monitor useful for controlling the
// kernel and exploring the system interactively
struct command {
    const char* name;
    const char* info;
    // return -1 to force monitor to exit;
    int32_t (*func)(struct trapframe* tf);
} commands[] = {
    { "help", "Display the list of commands.", cmd_help },
    { "info", "Display information about the kernel.", cmd_kernel_info },
    { "backtrace", "Print backtrace of stack frame.", cmd_backtrace }
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(struct command))

static int32_t run_cmd(char* cmd_name, struct trapframe* tf) {
    int i;
    for (i = 0; i < NUM_COMMANDS; i++)
        if (strcmp(cmd_name, commands[i].name) == 0)
            return commands[i].func(tf);
    putstr("Unknown command \'"); putstr(cmd_name); putstr("\'\n");
    return 0;
}

void monitor(struct trapframe* tf) {
    putstr("Kernel Debug Monitor:\n");
    putstr("Enter \'help\' for a list of commands.\n");
    
    char buffer[128];
    while (!(getstr(buffer) && run_cmd(buffer, tf) < 0)) 
        continue;
}

// cmd_help - print the information about commands
static int32_t cmd_help(struct trapframe* tf) {
    int32_t i;
    for (i = 0; i < NUM_COMMANDS; i++) {
        putstr(commands[i].name);
        putstr(" - ");
        putstr(commands[i].info);
        putch('\n');
    }
    return 0;
}

// cmd_kernel_info - call print_kernel_info() in kernel/debug/debug.c to
// print the memory occupancy in kernel.
static int32_t cmd_kernel_info(struct trapframe* tf) {
    print_kernel_info();
    return 0;
}

// cmt_backtrace - call print_stackframe in kernel/debug/debug.c to
// print a backtrace of th stack
static int32_t cmd_backtrace(struct trapframe* tf) {
    print_stack_frame();
    return 0;
}
