#include <stdio.h>
#include <time.h>

int main() {
    struct timespec ts;
    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret == 0) {
        printf("OK %ld\n", ts.tv_sec);
    } else {
        printf("BLOCKED\n");
    }
    return 0;
}
