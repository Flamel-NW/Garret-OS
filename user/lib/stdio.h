#ifndef __USER_LIB_USER_STDIO_H__
#define __USER_LIB_USER_STDIO_H__

#include "defs.h"

void putch(char c);
void putstr(const char* str);

void put_u64(u64 value, u8 base);
void put_i64(i64 value, u8 base);

#endif // __USER_LIB_USER_STDIO_H__