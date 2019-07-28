#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

void do1secloop(int divisor) {
    int i = 2;
    int j = i;
    for (; i < 500000000 / divisor; ++i)
        j += i;

    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    std::cout << (j - 1711656321 == 1 ? 1 : 0) << std::endl;
}

std::thread deepThread(int i, int threadsCount) {
    return std::thread{[i, threadsCount]() {
        if (i > 1) {
            auto th = deepThread(i - 1, threadsCount);
            do1secloop(threadsCount);
            th.join();
        }
        else {
            do1secloop(threadsCount);
        }
    }};
}

void deepTest(int threadsCount) {
    deepThread(threadsCount, threadsCount).join();
}

void flatTest(int threadsCount) {
    std::vector<std::thread> ths;
    for (int i = 0; i < threadsCount; ++i) {
        ths.emplace_back(
                std::thread{[threadsCount]() { do1secloop(threadsCount); }});
    }

    for (auto& th: ths) {
        th.join();
    }
}

int main(int argc, const char* argv[]) {
    if (argc != 3)
        return 1;

    int threadsCount = std::stoi(argv[2]);

    if (std::strcmp(argv[1], "deep") == 0) {
        deepTest(threadsCount);
    }
    else if (std::strcmp(argv[1], "flat") == 0) {
        flatTest(threadsCount);
    }
    else {
        return EXIT_FAILURE;
    }
}
