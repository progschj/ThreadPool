ThreadPool
==========

A simple C++11 Thread Pool implementation.

Basic usage:
```c++
// create thread pool with 4 worker threads
ThreadPool pool(4);

// enqueue and store future
auto result = pool.enqueue([](int answer) { return answer; }, 42);

// get result from future
std::cout << result.get() << std::endl;

```
## Build with CMake

This project includes a `CMakeLists.txt` for easy cross-platform building.

### Requirements

- C++11 compatible compiler (GCC 5+, Clang 3.3+, MSVC 2015+)
- CMake 3.10 or higher

### Build & Run

```bash
mkdir build
cd build
cmake ..
cmake --build .
./tp_main
```

### Direct Compilation (Linux/macOS)

```bash
g++ -std=c++11 -pthread example.cpp -o tp_main && ./tp_main
```

---
