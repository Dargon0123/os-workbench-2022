#include <stdlib.h>
#include <stdio.h>

int main() {
    int foo = 10, bar = 15;
# if 0
    asm volatile (
        "add %%rbx, %%rax"
        : "=a"(foo)
        : "a"(foo), "b"(bar)
    );
    printf("foo + bar = %d\r\n", foo);
#elif 1
    asm volatile (
        "lock; add %1, %0;" // lock: atomic operands
        : "=m"(foo)
        : "ir"(bar), "m"(foo)
    );
    printf("foo + bar = %d\r\n", foo);
#endif
}