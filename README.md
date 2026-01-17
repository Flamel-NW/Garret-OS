# Garret-OS

## 2025.12.29

重大改动！彻底修改uCore的原始内存布局

原本的uCore无论是清华的x86版本还是Liurunda的riscv64版本，实际上都有很多32位遗留的代码逻辑

我将其彻底的改进到64位，实现了真正的 64 位原生布局。（详见kernel/memlayout.h）

## 2023.5.2

基于uCore和xv6的risc-v64指令集的操作系统

- 网上现存的risc-v64 uCore都包含着大量uCore的原始代码，其中uCore又包含大量的原始xv6的祖传代码，对学习造成了很大困难，我打算写一个完全从0开始的riscv64的OS-kernel

- 做这个项目的目的是学习和深入理解操作系统的知识, 所以我给自己的要求不光是要手敲每一行代码, 而且还要理解每一行代码, 有疑惑的地方我会加比较详细的注释

- 因为不想copy代码, 所以lib里面的库函数我都实现的比较随意而且简陋, 和标准库很多接口都不同

- 也因此, 我会尽量减少无关代码, 尽量做到代码精简和清晰

- 算法选择: 

    - 页面分配算法: best_fit
    - 页面置换算法: FIFO (LRU 无法实现)

- 代码中出现"V I: P1", 这样的注释表示: 参见The RISC-V Instruction Set Manual Volume I Page 1

    手册版本:
    1. The RISC-V Instruction Set Manual

        Volume I: Unprivileged ISA

        Document Version 20191213
    
    2. The RISC-V Instruction Set Manual

        Volume II: Privileged Architecture

        Document Version 20211203

## TODO

- lab1中printfmt没有实现，就实现了个putstr（麻烦了点是麻烦了点但你就说能不能用吧）

- lab4中kmalloc, kfree相关内容以及SLOB分配器没实现 搞了个简单版本的malloc和free


## 构建教程

三年后重新捡起这个教程发现无从下手，发现还是得到处找 这次记录一下

1. 安装qemu, 安装交叉编译工具

    `apt install -y gcc-riscv64-unknown-elf qemu-system-riscv gdb_multiarch`

2. 编译

    `make build-$(用户程序名)`

3. 运行

    `make qemu`


4. 调试

    1.  - teminal 1: `make debug`

        - teminal 2: `gdb-multiarch Garret-OS` （之前是riscv64-unknown-elf-gdb）

            然后 `target remote :1234`
    
    2. vscode + codelldb: 参考launch.json和tasks.json

## 问题记录

### 2025.12.26 启动后一直在0x80000000前面点乱跑

```sh
(gdb) 0x0000000080200c92 in ?? ()

(gdb) 0x0000000080200c96 in ?? ()

(gdb) 0x0000000080200c9a in ?? ()

(gdb) 0x0000000080200c9e in ?? ()

(gdb) 0x0000000080200ca2 in ?? ()

(gdb) 0x0000000080200ca6 in ?? ()
...
```

entry.S 中必须写.text.kernel_entry不能写.text, 与kernel.ld中保持一致, 否则不会把kernel_entry放到最前面

### 2026.1.5 sret 跳转不到exit.c

修改某个地方的p_gpt应该是va而非pa，修改调试脚本加载用户程序符号表，不然就算跑到了也看不出来

### 2026.1.8

最开始的时候想着每个lab commit一次这太不现实 一次新增太多的代码 太多的功能 会导致完全没办法进行测试debug 以后应该模块化commit并测试

在 CS 里，这种做法叫 "Big Bang Integration" (大爆炸式集成)。这是软件工程教科书里反面教材的第一页：如果你一次性往系统里塞入 2000 行新代码、5 个新接口、3 个新数据库调用，当它 Crash 的时候，你根本无法定位是哪一行出了错。

### 2026.1.17

目前做完了user的exit和hello，别的问题不大了（应该吧）就先这样
