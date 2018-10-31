#include <stdio.h>

unsigned char bss[64UL * 1024 * 1024];

int main() {
    int n = 0;
    scanf("%d", &n);
    return bss[n];
}
