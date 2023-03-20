#ifndef __LIB_STRING_H__
#define __LIB_STRING_H__

#include "defs.h"

size_t strlen(const char* s);

char* strcpy(char* dst, char* src);

char* strrev(char* s);

void* memset(void* s, char c, size_t n);

#endif // __LIB_STRING_H__
