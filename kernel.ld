OUTPUT_ARCH(riscv)
ENTRY(kernel_entry)

BASE_ADDRESS = 0x80200000;
VIRTUAL_ADDRESS = 0xFFFFFFFF80200000;

SECTIONS 
{
    /* Load the kernel at this address: "." means the current address */
    . = VIRTUAL_ADDRESS;
    .text : AT (BASE_ADDRESS)
    {
        *(.text.kernel_entry)
        *(.text .text.*)
        *(.stub .gnu.linkonce.t.*)
    }
    PROVIDE(etext = .); /* Define the 'etext' symbol to this value */

    .rodata : 
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        *(.srodata .srodata.*)
    }

    /* Adjust the address for the data segment to the next page */
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
