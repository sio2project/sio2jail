#include <stdio.h>
#include <stdio.h>

int foo(long unsigned int* used) {
    if (*used >= 2ULL * 1024 * 1024 * 1024)
        return 0;

    *used += sizeof(int) * 1024;
    int tmp[1024];

    return tmp[foo(used) % 1024] + tmp[1023];
}

int main() {
    long unsigned int used = 0;
    int res = foo(&used);

    printf("FAIL used %lumb\n", used / 1024 / 1024, res);
    return 0;
}
