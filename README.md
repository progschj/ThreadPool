# ThreadPool

[![Build Status](https://travis-ci.org/WillBrennan/ThreadPool.svg?branch=master)](https://travis-ci.org/WillBrennan/ThreadPool)

A simple C++ Thread Pool using std::thread and other C++11 functionality.

This is a fork of [ThreadPool](https://github.com/progschj/ThreadPool) but a few extra features.


## Features
* must be explicitly constructed (no copy, assign, move, or move-assign construction)
* added more checks
* added tests!
* CMakeLists for both Clang and GCC

## Example

```c++
#include <iostream>
#include <vector>

#include "thread_pool.h"

int main(int argc, char** argv) {
    ThreadPool pool;
    std::vector<std::future<int>> results;

    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.enqueue([](int i) { return i * i; }, i));
    }

    for (auto&& result : results) {
        std::cout << result.get() << std::endl;
    }

    return 0;
}
```

## Getting Started

```bash
git clone https://github.com/WillBrennan/ThreadPool
mkdir -p ThreadPool/build && cd ThreadPool/build
cmake ../ && make && sudo make install
```

## Tests
[Google Test](https://github.com/google/googletest) must be installed to build the tests, tests are not build by default
due to install requirements. To run the tests;

```bash
git clone https://github.com/WillBrennan/ThreadPool
mkdir -p ThreadPool/build && cd ThreadPool/build
cmake -Dtests=ON ../ && make && make test
```

## Clang-Format
Project uses clang-format, call `make clang-format` before submitting a pull request.