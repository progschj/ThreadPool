#include <iostream>
#include <chrono>

#include "ThreadPool.h"

int main()
{
    
    std::future<int> last;
    {
        // the pool will discard pending tasks when exiting
        ThreadPool pool(2, false);
        
        auto task1 = []() -> int
        {
            std::cout << "Task 1" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return 1;
        };
        
        auto task2 = []() -> int
        {
            std::cout << "Task 2" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return 2;
        };
        
        auto task3 = []() -> int
        {
            std::cout << "Task 3" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
            return 3;
        };
        
        pool.enqueue(task1);
        pool.enqueue(task2);
        last = pool.enqueue(task3);
    }
    
    try
    {
        //when drain is false, future throws an exception if task was not completed
        std::cout << "===" << last.get() << "===" << std::endl;
    }
    catch( std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    
    return 0;
}
