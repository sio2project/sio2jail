#include <iostream>

int main() {
    int a, b;
    std::cin >> a >> b;
    int array[2] = {a, b};
    auto [c, d] = array;
    std::cout << c + d << std::endl;
    return 0;
}