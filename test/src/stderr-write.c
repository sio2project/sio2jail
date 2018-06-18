#include <unistd.h>

int main() {
    write(2, "stderr", 6);
    write(1, "stdout", 6);
    return 0;
}
