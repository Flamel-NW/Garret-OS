#ifndef __KERNEL_DRIVER_RISCV_H__
#define __KERNEL_DRIVER_RISCV_H__

#include "defs.h"


#define LOG_REGBYTES 3
#define REGBYTES (1 << LOG_REGBYTES)

#define SSTATUS_SIE 0x00000002          // V II: P64

// V II: P71
#define SCAUSE_INTR ((uint64_t) 1 << 63)

#define SCAUSE_SSI  (0x1 | SCAUSE_INTR) // Supervisor software interrupt
#define SCAUSE_STI  (0x5 | SCAUSE_INTR) // Supervisor timer interrupt
#define SCAUSE_SEI  (0x9 | SCAUSE_INTR) // Supervisor external interrupt    

#define SCAUSE_IAM  0x00000000          // Instruction address misaligned
#define SCAUSE_IAF  0x00000001          // Instruction access fault
#define SCAUSE_II   0x00000002          // Illegal instruction
#define SCAUSE_B    0x00000003          // Breakpoint
#define SCAUSE_LAM  0x00000004          // Load address misaligned
#define SCAUSE_LAF  0x00000005          // Load access fault
#define SCAUSE_SAM  0x00000006          // Store/AMO address misaligned
#define SCAUSE_SAF  0x00000007          // Store/AMO access fault
#define SCAUSE_ECFU 0x00000008          // Environment call from U-mode
#define SCAUSE_ECFS 0x00000009          // Environment call from S-mode
#define SCAUSE_IPF  0x0000000C          // Instruction page fault
#define SCAUSE_LPF  0x0000000D          // Load page fault
#define SCAUSE_SPF  0x0000000F          // Store/AMO page fault

// V II: P66
#define SIR_SSI (1 << (SCAUSE_SSI & ~SCAUSE_INTR))
#define SIR_STI (1 << (SCAUSE_STI & ~SCAUSE_INTR))
#define SIR_SEI (1 << (SCAUSE_SEI & ~SCAUSE_INTR))


// 语句表达式是C语言的一种特性, 它允许我们将多条语句组合成一个表达式, 
// 并且这个表达式的值就是最后一条语句的值

// 语句表达式通常用在需要返回多个值或需要执行多个语句但只能返回一个值的场景中通过语句表达式, 
// 我们可以在一个表达式中执行多个语句, 并将这些语句的结果封装为一个值返回

// 语句表达式的基本语法如下：
// ({ statement1; statement2; ... statementN; result_expression; })

// 其中, 大括号包裹的语句序列是语句表达式的实体, 最后的表达式就是语句表达式的值

// 下面这些宏函数用这种方法让宏函数能够返回值

// "%数字" 从0开始依次表示输出操作数和输入操作数, 如%0表示rd, %1表示rs1

// CSRR rd, csr - V I: P140
#define CSRR(csr) ({                \
    uint64_t rd;                    \
    __asm__ volatile (              \
        "csrr %0, " #csr            \
        : "=r" (rd)                 \
    );                              \
    rd;                             \
})

// CSRW csr, rs - V I: P140
#define CSRW(csr, rs) ({            \
    __asm__ volatile (              \
        "csrw " #csr ", %0"         \
        :: "r"(rs)                  \
    );                              \
})

// CSRRS rd, csr, rs1 - V I: P56
#define CSRRS(csr, rs1) ({          \
    uint64_t rd;                    \
    __asm__ volatile (              \
        "csrrs %0, " #csr ", %1"    \
        : "=r" (rd)                 \
        : "r" (rs1)                 \
    );                              \
    rd;                             \
})

// CSRRC rd, csr, rs1 - V I: P56
#define CSRRC(csr, rs1) ({          \
    uint64_t rd;                    \
    __asm__ volatile (              \
        "csrrc %0, " #csr ", %1"    \
        : "=r" (rd)                 \
        : "r" (rs1)                 \
    );                              \
    rd;                             \
})

// AMO rd, rs2, (rs1) - V I: P52
// "+" 代表该变量既作为输入也作为输出, 原值输入进行运算后输出值更新该值
// "A" 代表一个存放在通用寄存器中的地址

#define AMO_OP_D(op, rs2, rs1) ({   \
    uint64_t rd;                    \
    __asm__ volatile (              \
        "amo" #op ".d %0, %2, %1"   \
        : "=r" (rd), "+A" (rs1)     \
        : "r" (rs2)                 \
    );                              \
    rd;                             \
})


#endif // __KERNEL_DRIVER_RISCV_H__
