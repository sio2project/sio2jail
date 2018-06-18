#include <stdio.h>
#include <fcntl.h>

int main() {
    int i = 2;
    int j = i;
    for(;i<500000000;++i)
        j += i;

    int fd = open("score.txt", O_RDWR);

    /* ... */

    printf("%d\n", j-1711656321);
    return 0;
}
