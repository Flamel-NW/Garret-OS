#ifndef __LIB_STRING_H__
#define __LIB_STRING_H__

#include "defs.h"

size_t strlen(const char* s);

int32_t strcmp(const char* s1, const char* s2);

char* strcpy(char* dst, const char* src);

char* strrev(char* s);

void* memset(void* s, char c, size_t n);
void* memcpy(void* dst, const void* src, size_t n);

#endif // __LIB_STRING_H__
