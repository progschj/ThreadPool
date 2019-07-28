#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.hpp"

int main()
{
    const size_t ThreadNum = 4;
    const size_t TaskNum = 8;

    ThreadPool pool(ThreadNum);
    std::vector< std::future<int> > results;
    results.reserve(TaskNum);

    for(int i = 0; i < TaskNum; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    
    return 0;
}
