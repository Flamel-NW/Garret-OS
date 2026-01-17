#ifndef __LIB_STDIO_H__
#define __LIB_STDIO_H__

#include "defs.h"
#include "sbi.h"

void putch(char ch);

void putstr(const char* str);

char getch();

char* getstr(char* str);

#endif // __LIB_STDIO_H__
