#include "debug.h"
#include "defs.h"
#include "stdio.h"
#include "stdlib.h"

// print_kernel_info -- print the information about kernel, including the location
// of kernel entry, the start addresses of data and text segements, the start
// address of free memory and how many mamory that kernel has used.
void print_kernel_info() {
    extern char etext[], edata[], end[], kernel_init[];
    putstr("Special kernel symbols:\n");
    putstr("\tentry\t0x"); putstr(uptrtoa((uintptr_t) kernel_init, 16)); putstr("\t(virtual)\n");
    putstr("\tetext\t0x"); putstr(uptrtoa((uintptr_t) etext, 16)); putstr("\t(virtual)\n");
    putstr("\tedata\t0x"); putstr(uptrtoa((uintptr_t) edata, 16)); putstr("\t(virtual)\n");
    putstr("\tend\t0x"); putstr(uptrtoa((uintptr_t) end, 16)); putstr("\t(virtual)\n");
}

void print_stack_frame() {
    // 没写
}
