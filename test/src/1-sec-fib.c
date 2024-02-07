#include <stdlib.h>
#include <stdio.h>

volatile int n0 = 1000000000;

int main(int argc, char **argv) {
    //int n = atoi(argv[1]);
    //int n = 1000000000;
    int n = n0;
    int f_cur = 0;
    int f_next = 1;
    if (n & 1) {
        f_cur = 1;
        --n;
    }
    n /= 2;
    for (;n > 0; --n) {
        f_cur = f_cur + f_next;
        f_next = f_cur + f_next;
    }
    printf("%d\n", f_cur - 1532868155);
}
