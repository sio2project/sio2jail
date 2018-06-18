#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int main() {
    int res = 0;

    long unsigned int used = 0;
    do {
        used += 128 * 1024 * 1024;

        int* tmp = malloc(128 * 1024 * 1024);
        if (tmp == NULL)
            break;

        res += tmp[128 * 1024 * 1024 / sizeof(int) - 1];
    } while (used < 2ULL * 1024 * 1024 * 1024);

    printf("FAIL used %lumb\n", used / 1024 / 1024, res);
    return 0;
}
