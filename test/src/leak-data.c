#include <stdio.h>

unsigned char data[64UL * 1024 * 1024] = {1, 2, 3};

int main(int argc, const char* argv[]) {
    return data[((unsigned long)&argc) % (54 * 1024 * 1024 - 8) + 8];
}
