#ifndef __LIB_STDIO_H__
#define __LIB_STDIO_H__

#include "defs.h"
#include "sbi.h"

void putch(char ch);

void putstr(const char* str);

char getch();

char* getstr(char* str);

void put_u64(u64 value, u8 base);

void put_i64(i64 value, u8 base);

#endif // __LIB_STDIO_H__
