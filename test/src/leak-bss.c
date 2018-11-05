#include <stdio.h>

unsigned char bss[64UL * 1024 * 1024];

int main(int argc, const char* argv[]) {
    return bss[((unsigned long)&argc) % (64 * 1024 * 1024)];
}
