OUTPUT_ARCH(riscv)
ENTRY(kernel_entry)

BASE_ADDRESS = 0x80200000;
VIRTUAL_ADDRESS = 0xFFFFFFFFC0200000;

SECTIONS 
{
    /* Load the kernel at this address: "." means the current address */
    . = VIRTUAL_ADDRESS;
    .text : AT (BASE_ADDRESS)
    {
        *(.text.kernel_entry)
        *(.text)
        *(.stub)                
        *(.text.*)               
        *(.gnu.linkonce.t.*)
    }
    PROVIDE(etext = .); /* Define the 'etext' symbol to this value */

    .rodata : 
    {
        *(.rodata)
        *(.rodata.*)           
        *(.gnu.linkonce.t.*)
    }

    /* Adjust the address for the data segment to the next page */
    . = ALIGN(0x1000); 

    /* The data segment */
    .data : 
    {
        *(.data)
        *(.data.*)
    }

    .sdata : 
    {
        *(.sdata)
        *(.sdata.*)
    }
    PROVIDE(edata = .); /* Define the 'edata' symbol to this value */

    .bss : 
    {
        *(.bss)
        *(.bss.*)
        *(.sbss*)
    }
    PROVIDE(end = .); /* Define the 'end' symbol to this value */

    /DISCARD/ : {
        *(eh_frame, .note.GNU-stack)
    }
}
