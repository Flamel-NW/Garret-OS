#include "string.h"

#include "defs.h"

size_t strlen(const char* s) {
    size_t cnt = 0;
    while (*s++ != '\0')
        cnt++;
    return cnt;
}

int32_t strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2)
        s1++, s2++;
    return (int32_t) (*s1 - *s2);
}

char* strcpy(char* dst, const char* src) {
    char* p = dst;
    while ((*p++ = *src++) != '\0')
        continue;
    return dst;
}

char* strrev(char* s) {
    size_t len = strlen(s);
    for (size_t i = 0; i < len / 2; i++) {
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

void* memcpy(void* dst, const void* src, size_t n) {
    const byte* s = src;
    byte* p = dst;
    while (n--) 
        *p++ = *s++;
    return dst;
}
