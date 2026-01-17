#ifndef __LIB_DEFS_H__
#define __LIB_DEFS_H__

#define NULL ((void*) 0)

#define INT8_MIN    (-0x7f - 1)
#define INT16_MIN   (-0x7fff - 1)
#define INT32_MIN   (-0x7fffffff - 1)
#define INT64_MIN   (-0x7fffffffffffffff - 1)

#define INT8_MAX    (0x7f)
#define INT16_MAX   (0x7fff)
#define INT32_MAX   (0x7fffffff)
#define INT64_MAX   (0x7fffffffffffffff)

#define UINT8_MAX   (0xff)
#define UINT16_MAX  (0xffff)
#define UINT32_MAX  (0xffffffff)
#define UINT64_MAX  (0xffffffffffffffff)

#ifndef __ASSEMBLER__
// 下面代码只在C源文件中展开(汇编源文件中不能识别typedef定义, 会报错)

typedef _Bool bool;
#define true 1
#define false 0

// Explicitly-sized versions of integer types 
// Rust Style
typedef signed char i8;                 
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;

static inline u64 rounddown(u64 a, u64 n) {
    return (a / n) * n;
}

static inline u64 roundup(u64 a, u64 n) {
    return ((a + n - 1) / n) * n;
}

#endif // __ASSEMBLER__


// Return the offset of "membor" relative to the beginning of a struct type
#define OFFSET_OF(type, member)                                 \
    ((u64) (&((type*) 0)->member))

// TO_STRUCT - get the struct from a ptr of member
#define TO_STRUCT(type, member, ptr)                            \
    ((type*) ((u8*) (ptr) - OFFSET_OF(type, member)))


#endif // __LIB_DEFS_H__
