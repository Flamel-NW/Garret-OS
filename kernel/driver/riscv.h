#ifndef __LIB_RISCV_H__
#define __LIB_RISCV_H__

#include "defs.h"

// "V I: P1" means "See The RISC-V Instruction Set Manual Volume I Page 1."

#define SSTATUS_SIE 0x00000002  // V II: P64

// temp变量应该是单纯占位用的
// %数字从0开始依次表示输出操作数和输入操作数, 如%0表示temp, %1表示rs1

// CSRRS rd, csr, rs1. V I: P56
#define CSRRS(csr, rs1) ({          \
    uint64_t temp;                  \
    asm volatile (                  \
        "csrrs %0, " #csr ", %1"    \
        : "=r" (temp)               \
        : "r" (rs1)                 \
    );                              \
})

// CSRRC rd, csr, rs1. V I: P56
#define CSRRC(csr, rs1) ({          \
    uint64_t temp;                  \
    asm volatile (                  \
        "csrrc %0, " #csr ", %1"    \
        : "=r" (temp)               \
        : "r" (rs1)                 \
    );                              \
})

#endif // __LIB_RISCV_H__
