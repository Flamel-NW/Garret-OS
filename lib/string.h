#ifndef __LIB_STRING_H__
#define __LIB_STRING_H__

#include "defs.h"

u64 strlen(const char* s);

i32 strcmp(const char* s1, const char* s2);

char* strcpy(char* dst, const char* src);

char* strrev(char* s);

void* memset(void* s, char c, u64 n);
void* memcpy(void* dst, const void* src, u64 n);

#endif // __LIB_STRING_H__
