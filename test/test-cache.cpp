#include <iostream>
#include <vector>
#include <chrono>
#include <future>
#include "../ThreadPool.hpp"


int main() {
    ThreadPool pool(0, 8);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 16; ++i) {
        results.emplace_back(
            pool.Schedule([i] {
                std::cout << "hello " << i << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << "\n";
                return i * i;
            })
        );
    }

    for (auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(61));
    return 0;
}
