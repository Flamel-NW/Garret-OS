#include "stdio.h"

i32 zero;

i32 main() {
    putstr("value is "); put_i64(1 / zero, 16); putstr(" (should be -1 in RISC-V).\n");
    putstr("divzero pass.\n");
    return 0;
}
