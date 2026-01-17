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
    extern u8 etext[], edata[], end[], init_kernel[];
    putstr("Special kernel symbols:\n");
    putstr("\tentry\t\t0x"); putstr(u64toa((u64) init_kernel, 16)); putstr("\t(virtual)\n");
    putstr("\tetext\t\t0x"); putstr(u64toa((u64) etext, 16)); putstr("\t(virtual)\n");
    putstr("\tedata\t\t0x"); putstr(u64toa((u64) edata, 16)); putstr("\t(virtual)\n");
    putstr("\tend\t\t0x"); putstr(u64toa((u64) end, 16)); putstr("\t(virtual)\n\n");
}

void print_memory_info() {
    putstr("Memory layout:\n");

    putstr("\tDRAM_BEGIN_PA\t0x80000000\n");
    putstr("\tDRAM_BEGIN_VA\t0xFFFFFFFFC0000000\n");

    putstr("\tDRAM_END_PA\t0x88000000\n");
    putstr("\tDRAM_END_VA\t0xFFFFFFFFC8000000\n");

    putstr("\tKERNEL_BEGIN_PA\t0x80200000\n");
    putstr("\tKERNEL_BEGIN_VA\t0xFFFFFFFFC0200000\n");

    putstr("\tKERNEL_END_PA\t0x82000000\n");
    putstr("\tKERNEL_END_VA\t0xFFFFFFFFC2000000\n");

    putstr("\tUSER_BEGIN_PA\t0x82000000\n");
    putstr("\tUSER_BEGIN_VA\t0xFFFFFFFFC2000000\n");

    putstr("\tUSER_END_PA\t0x88000000\n");
    putstr("\tUSER_END_VA\t0xFFFFFFFFC8000000\n\n");
}

void print_pages_info () {
    putstr("Pages infomation:\n");

    putstr("\tg_pages begin\n"); 
    putstr("\t\t\t0x"); putstr(u64toa((u64) g_pages, 16)); putstr("\t(virtual)\n");
    
    putstr("\tg_pages end\n"); 
    putstr("\t\t\t0x"); 
    putstr(u64toa((u64) (g_pages + g_num_qemu_pages + g_num_kernel_pages + g_num_free_pages), 16));
    putstr("\t(virtual)\n");
    
    putstr("\tnumber of qemu-reserved pages (0 ~ DRAM_BEGIN)\n");
    putstr("\t\t\t"); putstr(u64toa((u64) g_num_qemu_pages, 10)); putch('\n');

    putstr("\tnumber of kernel-reserved pages (DRAM_BEGIN ~ KERNEL_END)\n");
    putstr("\t\t\t"); putstr(u64toa((u64) g_num_kernel_pages, 10)); putch('\n');

    putstr("\tnumber of free pages (USER_BEGIN ~ USER_END)\n");
    putstr("\t\t\t"); putstr(u64toa((u64) g_num_free_pages, 10)); 
    putstr("\n\n");
}

