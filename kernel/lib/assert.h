#ifndef __LIB_ASSERT_H__
#define __LIB_ASSERT_H__

#include "defs.h"

void warn(const char* file, const char* func, i32 line, const char* str);

void panic(const char* file, const char* func, i32 line, const char* str) __attribute__((noreturn));

#define WARN(str) do warn(__FILE__, __func__, __LINE__, str); while(0)

#define PANIC(str) do panic(__FILE__, __func__, __LINE__, str); while(0)

#define ASSERT(x) do if(!(x)) PANIC("assertion failed: " #x); while(0)
    

#endif // __LIB_ASSERT_H__
