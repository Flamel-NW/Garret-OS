#ifndef __KERNEL_DRIVER_RISCV_H__
#define __KERNEL_DRIVER_RISCV_H__

#define LOG_REGBYTES 3
#define REGBYTES (1 << LOG_REGBYTES)

#define SSTATUS_SIE 0x0000000000000002  // V II: P64

// V II: P71
#define SCAUSE_SSI  0x8000000000000001  // Supervisor software interrupt
#define SCAUSE_STI  0x8000000000000005  // Supervisor timer interrupt
#define SCAUSE_SEI  0x8000000000000009  // Supervisor external interrupt    

#define SCAUSE_IAM  0x0000000000000000  // Instruction address misaligned
#define SCAUSE_IAF  0x0000000000000001  // Instruction access fault
#define SCAUSE_II   0x0000000000000002  // Illegal instruction
#define SCAUSE_B    0x0000000000000003  // Breakpoint
#define SCAUSE_LAM  0x0000000000000004  // Load address misaligned
#define SCAUSE_LAF  0x0000000000000005  // Load access fault
#define SCAUSE_SAM  0x0000000000000006  // Store/AMO address misaligned
#define SCAUSE_SAF  0x0000000000000007  // Store/AMO access fault
#define SCAUSE_ECFU 0x0000000000000008  // Environment call from U-mode
#define SCAUSE_ECFS 0x0000000000000009  // Environment call from S-mode
#define SCAUSE_IPF  0x000000000000000C  // Instruction page fault
#define SCAUSE_LPF  0x000000000000000D  // Load page fault
#define SCAUSE_SPF  0x000000000000000F  // Store/AMO page fault

// V II: P66
#define SIR_SSI (1 << (SCAUSE_SSI & 0x7FFFFFFFFFFFFFFF))
#define SIR_STI (1 << (SCAUSE_STI & 0x7FFFFFFFFFFFFFFF))
#define SIR_SEI (1 << (SCAUSE_SEI & 0x7FFFFFFFFFFFFFFF))

// %数字从0开始依次表示输出操作数和输入操作数, 如%0表示temp, %1表示rs1

// CSRW csr, rs - V I: P139
#define CSRW(csr, rs) ({            \
    asm volatile (                  \
        "csrw " #csr ", %0"         \
        :: "r"(rs)                  \
    );                              \
})

// temp变量应该是单纯占位用的
// CSRRS rd, csr, rs1 - V I: P56
#define CSRRS(csr, rs1) ({          \
    uint64_t temp;                  \
    asm volatile (                  \
        "csrrs %0, " #csr ", %1"    \
        : "=r" (temp)               \
        : "r" (rs1)                 \
    );                              \
})

// CSRRC rd, csr, rs1 - V I: P56
#define CSRRC(csr, rs1) ({          \
    uint64_t temp;                  \
    asm volatile (                  \
        "csrrc %0, " #csr ", %1"    \
        : "=r" (temp)               \
        : "r" (rs1)                 \
    );                              \
})

#endif // __KERNEL_DRIVER_RISCV_H__
