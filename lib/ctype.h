#ifndef __LIB_CTYPE_H__
#define __LIB_CTYPE_H__

#include "defs.h"

static inline bool isdigit(char ch) {
    return ch >= '0' && ch <= '9';
}

static inline bool islower(char ch) {
    return ch >= 'a' && ch <= 'z';
}

static inline bool isupper(char ch) {
    return ch >= 'A' && ch <= 'Z';
}

static inline bool isalpha(char ch) {
    return islower(ch) || isupper(ch);
}

static inline bool isalnum(char ch) {
    return isdigit(ch) || isalpha(ch);
}

static inline bool ispunct(char ch) {
    return (ch >= '!' && ch <= '/') ||
        (ch >= ':' && ch <= '@') ||
        (ch >= '[' && ch <= '`') ||
        (ch >= '{' && ch <= '~');
}

static inline bool isgraph(char ch) {
    return isalnum(ch) || ispunct(ch);
}

static inline bool issapce (char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' ||
        ch == '\v' || ch == 'f' || ch == '\r';
}

#endif // __LIB_CTYPE_H__
