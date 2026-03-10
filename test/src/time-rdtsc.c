#include <stdio.h>
#include <stdint.h>

int main() {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    uint64_t tsc = ((uint64_t)hi << 32) | lo;
    printf("TSC %lu\n", tsc);
    return 0;
}
