# Garret-OS

基于uCore和xv6的risc-v64指令集的操作系统

- 做这个项目的目的是学习和深入理解操作系统的知识, 所以我给自己的要求不光是要手敲每一行代码, 而且还要理解每一行代码, 有疑惑的地方我会加比较详细的注释

- 因为不想copy代码, 所以lib里面的库函数我都实现的比较随意而且简陋, 和标准库很多接口都不同

- 也因此, 我会尽量减少无关代码

- 代码中出现"V I: P1", 这样的注释表示: 参见The RISC-V Instruction Set Manual Volume I Page 1

    手册版本:
    1. The RISC-V Instruction Set Manual

        Volume I: Unprivileged ISA

        Document Version 20191213
    
    2. The RISC-V Instruction Set Manual

        Volume II: Privileged Architecture

        Document Version 20211203

## TODO:

- ./kernel/mm: buddy system
