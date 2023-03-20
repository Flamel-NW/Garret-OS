/**
 * @file kelnel.ld
 * @author Flamel-NW (flamel.nw@qq.com)
 * @brief Simple linker script for Garret-OS kernel.
 *      See the GNU ld 'info' manual ("info ld") to leran the syntax.
 * @date 2023-03-18
 */

OUTPUT_ARCH(riscv)
ENTRY(kernel_entry)

BASE_ADDRESS = 0x80200000;

SECTIONS
{
    /* Load the kernel at this address: "." means the current address */
    . = BASE_ADDRESS;
    .text : {
        *(.text.kernel_entry)
        *(.text .stub .text.* .gnu.linkonce.t.*)
    }
    PROVIDE(etext = .); /* Define the 'etext' symbol to this value */

    .rodata : {
        *(.rodata, .rodata.* .gnu.linkonce.t.*)
    }

    /* Adjust the address for the data segment to the next page */
    . = ALIGN(0x1000); 

    /* The data segment */
    .data : {
        *(.data)
        *(.data.*)
    }

    .sdata : {
        *(.sdata)
        *(.sdata.*)
    }
    PROVIDE(edata = .); /* Define the 'edata' symbol to this value */

    .bss : {
        *(.bss)
        *(.bss.*)
        *(.sbss*)
    }
    PROVIDE(end = .); /* Define the 'end' symbol to this value */

    /DISCARD/ : {
        *(eh_frame, .note.GNU-stack)
    }
}
