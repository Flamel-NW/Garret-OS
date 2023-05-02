    .text

    # void kernel_proc_entry();
    .global kernel_proc_entry
kernel_proc_entry:
    move a0, s1
    jalr s0

    jal do_exit
