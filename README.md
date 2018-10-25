ThreadPool
==========

A simple C++11 Dynamic Thread Pool implementation.

Basic usage:
```c++
// create thread pool with 4 worker threads
ThreadPool pool(4);

// enqueue and store future
auto result = pool.Schedule([](int answer) { return answer; }, 42);

// get result from future
std::cout << result.get() << std::endl;


int secs = 5;   // closure.
// don't need result.
pool.Enqueue([=]() { std::this_thread::sleep_for(std::chrono::seconds(secs)); });


// dynamic useage.
// init a pool has only one thread, and automatic expands.
auto conc = std::thread::hardware_concurrency() * 2;
ThreadPool dp(0, conc);

for (int i = 0; i < 20; ++i)
{
    //dp.enqueue([]() { std::this_thread::sleep_for(500ms); });    // C++14
    dp.Enqueue([]() { std::this_thread::sleep_for(std::chrono::milliseconds(500)); });
}

```
