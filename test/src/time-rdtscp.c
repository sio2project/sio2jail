#include <stdio.h>
#include <stdint.h>

int main() {
    uint32_t lo, hi, aux;
    __asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
    uint64_t tsc = ((uint64_t)hi << 32) | lo;
    printf("TSC %lu AUX %u\n", tsc, aux);
    return 0;
}
