#include <stdio.h>

int main() {
    int i = 2;
    int j = i;
    for(;i<500000000;++i) {
        j += i;
        // Keep the loop at ~4 instructions per iteration regardless of
        // compiler optimizations (newer gcc merges two iterations into one).
        __asm__ volatile("" : "+r"(j));
    }

    printf("%d\n", j-1711656321);
    return 0;
}
