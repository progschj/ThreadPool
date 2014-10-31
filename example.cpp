#include <iostream>
#include <chrono>

#include "ThreadPool.h"

int main()
{
    
    std::future<int> first, second, last;
    {
        // the pool will execute all pending tasks before exiting
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
        
        first = pool.enqueue(task1);
        second = pool.enqueue(task2);
        last = pool.enqueue(task3);
        
        // The pool will destroy immediately after queueing tasks, so there is no gurantee which tasks will get to run on the threads, when drain is set to FALSE.
        // Use this sleep to delay this. Ugly, but effective for demo purposes
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    try
    {
        std::cout << "===" << first.get() << "===" << std::endl;
        std::cout << "===" << second.get() << "===" << std::endl;
        std::cout << "===" << last.get() << "===" << std::endl;
    }
    catch( std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    
    return 0;
}
