#include <stdio.h>

int main() {
    int sum = 0;
    for (int i = 0; i < 1000; i++)
        sum += i;
    printf("OK %d\n", sum);
    return 0;
}
