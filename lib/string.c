#include "string.h"
#include "defs.h"

size_t strlen(const char* s) {
    size_t cnt = 0;
    while (*s++ != '\0')
        cnt++;
    return cnt;
}

char* strcpy(char* dst, char* src) {
    char* p = dst;
    while ((*p++ = *src++) != '\0')
        continue;
    return dst;
}

char* strrev(char* s) {
    size_t len = strlen(s);
    size_t i;
    for (i = 0; i < len / 2; i++) {
        char temp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = temp;
    }
    return s;
}

void* memset(void* s, char c, size_t n) {
    char* p = s;
    while (n-- > 0) 
        *p++ = c;
    return s;
}
