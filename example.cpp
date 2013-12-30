#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int main()
{
    
    ThreadPool pool;
    std::vector< std::future<int> > results;
    
    std::cout << "Number of workers: " << pool.noWorkers() << std::endl;

    for(int i = 0; i < 8; ++i) {
        results.push_back(
            pool.enqueue([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }   
    
    for(size_t i = 0;i<results.size();++i)
        std::cout << results[i].get() << ' ';
    std::cout << std::endl;
    
    return 0;
}
