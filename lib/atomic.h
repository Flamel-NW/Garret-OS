#ifndef __LIB_ATOMIC_H__
#define __LIB_ATOMIC_H__

#include "defs.h"
#include "riscv.h"

// set_bit - Atomically set a bit in memory and return its old value
static inline bool set_bit(volatile void* base, uint64_t offset) {
    uint64_t mask = ((uint64_t) 1) << (offset % 64);
    return AMO_OP_D(or, mask, ((volatile uint64_t*) base)[offset / 64]) & mask;
}

// clear_bit - Atomically clear a bit in memory and return its old value
static inline bool clear_bit(volatile void* base, uint64_t offset) {
    uint64_t mask = ~(((uint64_t) 1) << (offset % 64));
    return AMO_OP_D(and, mask, ((volatile uint64_t*) base)[offset / 64]) & mask;
}

// change_bit - Atomically toggle a bit in memory and return its old value
static inline bool change_bit(volatile void* base, uint64_t offset) {
    uint64_t mask = ((uint64_t) 1) << (offset % 64);
    return AMO_OP_D(xor, mask, ((volatile uint64_t*) base)[offset / 64]);
}

// test_bit - Atomically determine whether a bit is set
static inline bool test_bit(volatile void* base, uint64_t offset) {
    return (((*(volatile uint64_t*) base) >> offset) & 1);
}

#endif // __LIB_ATOMIC_H__
