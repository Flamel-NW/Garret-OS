#ifndef __LIB_STDLIB_H__
#define __LIB_STDLIB_H__

#include "defs.h"

char ui8toc(uint8_t value);

char* i32toa(int32_t value, char* buffer, uint8_t base);

char* i64toa(int64_t value, char* buffer, uint8_t base);
char* ui64toa(uint64_t value, char* buffer, uint8_t base);

char* ptrtoa(intptr_t value, char* buffer, uint8_t base);

char* uptrtoa(uintptr_t value, char* buffer, uint8_t base);

#endif // __LIB_STDIO_H__
