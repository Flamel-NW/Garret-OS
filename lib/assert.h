#ifndef __LIB_ASSERT_H__
#define __LIB_ASSERT_H__

#include "defs.h"

void warn(const char* file, const char* func, int32_t line, const char* str);

void panic(const char* file, const char* func, int32_t line, const char* str) __attribute__((noreturn));

#define WARN(str) warn(__FILE__, __func__, __LINE__, str);

#define PANIC(str) panic(__FILE__, __func__, __LINE__, str);

#define ASSERT(x) do if(!(x)) PANIC("assertion failed: " #x) while(0)
    

#endif
