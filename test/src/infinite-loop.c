#include <stdio.h>

int main() {
    int i = 2;
    int j = i;
    for(;;++i)
        j += i;

    printf("%d\n", j-1711656321);
    return 0;
}
