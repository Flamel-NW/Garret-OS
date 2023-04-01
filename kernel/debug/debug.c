#include "debug.h"
#include "defs.h"
#include "memlayout.h"
#include "stdio.h"
#include "stdlib.h"
#include "pmm.h"


// print_kernel_info -- print the information about kernel, including the location
// of kernel entry, the start addresses of data and text segements, the start
// address of free memory and how many mamory that kernel has used.
void print_kernel_info() {
    extern char etext[], edata[], end[], init_kernel[];
    putstr("Special kernel symbols:\n");
    putstr("\tentry\t\t0x"); putstr(uptrtoa((uintptr_t) init_kernel, 16)); putstr("\t(virtual)\n");
    putstr("\tetext\t\t0x"); putstr(uptrtoa((uintptr_t) etext, 16)); putstr("\t(virtual)\n");
    putstr("\tedata\t\t0x"); putstr(uptrtoa((uintptr_t) edata, 16)); putstr("\t(virtual)\n");
    putstr("\tend\t\t0x"); putstr(uptrtoa((uintptr_t) end, 16)); putstr("\t(virtual)\n\n");
}

void print_memory_info() {
    putstr("Memory layout:\n");

    putstr("\tDRAM_BASE_PA\t0x80000000\n");
    putstr("\tDRAM_BASE_VA\t0xFFFFFFFFC0000000\n");

    putstr("\tDRAM_END_PA\t0x88000000\n");
    putstr("\tDRAM_END_VA\t0xFFFFFFFFC8000000\n");

    putstr("\tKERNEL_BEGIN_PA\t0x80200000\n");
    putstr("\tKERNEL_BEGIN_VA\t0xFFFFFFFFC0200000\n");

    putstr("\tKERNEL_END_PA\t0x81000000\n");
    putstr("\tKERNEL_END_VA\t0xFFFFFFFFC1000000\n");

    putstr("\tFREE_BEGIN_PA\t0x81000000\n");
    putstr("\tFREE_BEGIN_VA\t0xFFFFFFFFC1000000\n");

    putstr("\tFREE_END_PA\t0x88000000\n");
    putstr("\tFREE_END_VA\t0xFFFFFFFFC8000000\n\n");
}

void print_pages_info () {
    putstr("Pages infomation:\n");

    putstr("\tkernel-reserved pages begin\n");
    putstr("\t\t\t0x"); putstr(uptrtoa((uintptr_t) g_kernel_pages, 16)); putstr("\t(virtual)\n");
    
    putstr("\tnumber of kernel-reserved pages\n");
    putstr("\t\t\t"); putstr(uptrtoa((uintptr_t) g_num_kernel_pages, 10)); putch('\n');

    putstr("\tkernel(-reserved pages) end & free memory/pages begin\n");
    putstr("\t\t\t0x"); putstr(uptrtoa((uintptr_t) KERNEL_END_VA, 16)); putstr("\t(virtual)\n");

    putstr("\tnumber of free pages\n");
    putstr("\t\t\t"); putstr(uptrtoa((uintptr_t) FREE_PAGES, 10)); putch('\n');

    putstr("\tfree memory/pages end\n");
    putstr("\t\t\t0x"); putstr(uptrtoa((uintptr_t) FREE_END_VA, 16)); putstr("\t(virtual)\n\n");
}

void print_stack_frame() {
    // 没写
}
