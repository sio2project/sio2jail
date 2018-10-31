#include <stdio.h>

unsigned char data[64UL * 1024 * 1024] = {1, 2, 3};

int main() {
    int n = 0;
    scanf("%d", &n);
    return data[n];
}
