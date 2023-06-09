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

typedef unsigned char byte;
typedef unsigned char bool;

/* Explicitly-sized versions of integer types */
typedef signed char int8_t;                 
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

/* *
 * Pointers and addresses are 32 bits long.
 * We use pointer types to represent addresses,
 * uintptr_t to represent the numerical values of addresses.
 * */
typedef int64_t intptr_t;
typedef uint64_t uintptr_t;

/* size_t is used for memory object sizes */
typedef uintptr_t size_t;

static inline size_t rounddown(size_t a, size_t n) {
    return (a / n) * n;
}

static inline size_t roundup(size_t a, size_t n) {
    return ((a + n - 1) / n) * n;
}

#endif // __ASSEMBLER__


// Return the offset of "membor" relative to the beginning of a struct type
#define OFFSET_OF(type, member)                                 \
    ((size_t) (&((type*) 0)->member))

// TO_STRUCT - get the struct from a ptr of member
#define TO_STRUCT(type, member, ptr)                            \
    ((type*) ((byte*) (ptr) - OFFSET_OF(type, member)))


#endif // __LIB_DEFS_H__
