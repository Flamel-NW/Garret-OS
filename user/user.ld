OUTPUT_ARCH(riscv)
ENTRY(user_entry)

SECTIONS {
    /* Load programs at this address: "." means the current address */
    /* #define USER_BEGIN          0x00200000 */
    . = 0x00200000;

    .text :
    {
        *(.text.user_entry)
        *(.text .text.*)
        *(.stub .gnu.linkonce.t.*)
    }

    .rodata : 
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        *(.srodata .srodata.*)
    }

    /* Adjust the address for the data sagment to the next page */
    . = ALIGN(0x1000);

    /* The data segment */
    .data : 
    {
        *(.data .data.*)
    }

    .sdata : 
    {
        *(.sdata .sdata.*)
    }
    PROVIDE(edata = .); /* Define the 'edata' symbol to this value */

    .sbss : {
        *(.sbss .sbss.*)
    }

    .bss : 
    {
        *(.bss .bss.*)
    }
    PROVIDE(end = .); /* Define the 'end' symbol to this value */

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
        *(.comment)
    }
}
