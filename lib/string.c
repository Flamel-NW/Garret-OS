#include "string.h"

#include "defs.h"

u64 strlen(const char* s) {
    u64 cnt = 0;
    while (*s++ != '\0')
        cnt++;
    return cnt;
}

i32 strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2)
        s1++, s2++;
    return (i32) (*s1 - *s2);
}

char* strcpy(char* dst, const char* src) {
    char* p = dst;
    while ((*p++ = *src++) != '\0')
        continue;
    return dst;
}

char* strrev(char* s) {
    u64 len = strlen(s);
    for (u64 i = 0; i < len / 2; i++) {
        char temp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = temp;
    }
    return s;
}

void* memset(void* s, char c, u64 n) {
    char* p = s;
    while (n-- > 0) 
        *p++ = c;
    return s;
}

void* memcpy(void* dst, const void* src, u64 n) {
    const u8* s = src;
    u8* p = dst;
    while (n--) 
        *p++ = *s++;
    return dst;
}
