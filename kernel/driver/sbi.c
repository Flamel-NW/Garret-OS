#include "sbi.h"

uint16_t SBI_SET_TIMER = 0;
uint16_t SBI_CONSOLE_PUTCHAR = 1;
uint16_t SBI_CONSOLE_GETCHAR = 2;
uint16_t SBI_CLEAR_IPI = 3;
uint16_t SBI_SEND_IPI = 4;
uint16_t SBI_REMOTE_FENCE_I = 5;
uint16_t SBI_REMOTE_SFENCE_VMA = 6;
uint16_t SBI_REMOTE_SFENCE_VMA_ASID = 7;
uint16_t SBI_SHUTDOWN = 8;

// __asm__ volatile(
// 	汇编指令列表
// 	∶输出操作数 //非必需
// 	∶输入操作数 //非必需
// 	∶可能影响的寄存器或存储器 //非必需
// );

static uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uint64_t ret;
    __asm__ volatile (
        "mv x17, %[sbi_type]\n"
        "mv x10, %[arg0]\n"
        "mv x11, %[arg1]\n"
        "mv x12, %[arg2]\n"     // mv操作把参数的数值放到寄存器里
        "ecall\n"               // 参数放好之后通过ecall, 交给OpenSBI来执行
        // OpenSBI按照risc-v的calling convertion, 把返回值放到x10寄存器里
        // 我们还需要自己通过内联汇编把返回值拿到我们的变量里

        // 字母"r"表示使用编译器自动分配的寄存器来存储该操作数变量
        // 对于"输出操作数"而言, "="代表变量用作输出, 原来的值会被新值替换    
        "mv %[ret], x10"
        : [ret] "=r" (ret)  // 将mv指令的目标操作数ret_val和C程序中的变量ret_val绑定
        : [sbi_type] "r" (sbi_type), [arg0] "r" (arg0), [arg1] "r" (arg1), [arg2] "r" (arg2) // 同理
        
        // 如果内联汇编中的某个指令以无法预料的形式修改了存储器中的值, 
        // 则必须在__asm__中第三个冒号后的“可能影响的寄存器或存储器”中显示地加上“memory”, 
        // 从而通知GCC编译器不要将存储器中的值暂存在处理器的通用寄存器中。
        : "memory"
    );
    return ret;
}

void sbi_console_putchar(uint64_t ch) {
    sbi_call(SBI_CONSOLE_PUTCHAR, ch, 0, 0);
}

uint64_t sbi_console_getchar() {
    return sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0);
}

void sbi_set_timer(uint64_t stime) {
    sbi_call(SBI_SET_TIMER, stime, 0, 0);
}
