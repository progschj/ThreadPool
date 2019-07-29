#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int main()
{
    
    ThreadPool pool(4);
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
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

    // test params
    int value1 = 1;
    int value2 = 1;
    int value = pool.enqueue([](int v1, int& v2){
        ++v1;
        ++v2;
        return v1 + v2;
    }, value1, value2).get();
    std::cout << "value1:" << value1 << std::endl;  // 1
    std::cout << "value2:" << value2 << std::endl;  // 2
    std::cout << "value:"  << value  << std::endl;  // 4

    return 0;
}
