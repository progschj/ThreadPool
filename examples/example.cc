#include <iostream>
#include <vector>

#include "thread_pool.h"

int main(int argc, char** argv) {
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.enqueue([](auto i) { return i * i; }, i));
    }

    for (auto&& result : results) {
        std::cout << result.get() << std::endl;
    }

    return 0;
}
