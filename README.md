# ThreadPool

[![Build Status](https://travis-ci.org/WillBrennan/ThreadPool.svg?branch=master)](https://travis-ci.org/WillBrennan/ThreadPool)

A simple C++ Thread Pool using std::thread and other C++11/14 functionality.

```c++
#include <iostream>
#include <vector>

#include "thread_pool.h"

int main(int argc, char** argv) {
    ThreadPool pool;
    std::vector<std::future<int>> results;

    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.enqueue([](auto i) { return i * i; }, i));
    }

    for (auto&& result : results) {
        std::cout << result.get() << std::endl;
    }

    return 0;
}
```