#include <stdio.h>
#include <stdint.h>

static uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

int main() {
    uint64_t a = rdtsc();
    uint64_t b = rdtsc();
    uint64_t diff = (b > a) ? (b - a) : (a - b);
    // Real RDTSC: b > a always, diff ~20-100 cycles.
    // Random emulation: unrelated values, diff typically millions+.
    if (b < a || diff > 10000) {
        printf("RANDOM %lu %lu\n", a, b);
    } else {
        printf("MONOTONIC %lu %lu\n", a, b);
    }
    return 0;
}
