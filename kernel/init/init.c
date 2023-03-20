#include "defs.h"
#include "stdio.h"
#include "string.h"
#include "debug.h"

int32_t kernel_init() __attribute__((noreturn));

int kernel_init() {
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    putstr("Garret-OS is loading...\n");

    print_kernel_info();

    while (1)
        continue;

}
