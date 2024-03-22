ThreadPool
==========

A simple C++11 Thread Pool implementation.

Basic usage:
```c++
// 创建4个线程组成的线程池
// create thread pool with 4 worker threads
ThreadPool pool(4);

// enqueue and store future
auto result = pool.enqueue([](int answer) { return answer; }, 42);

// get result from future
std::cout << result.get() << std::endl;

```
