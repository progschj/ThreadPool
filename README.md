ThreadPool
==========

A simple C++11 Thread Pool implementation.

Basic usage:
```c++
// create thread pool
ThreadPool pool;

// enqueue and store future
auto result = pool.enqueue([](int answer) { return answer; }, 42);

// get result from future
std::cout << result.get() << std::endl;

```
