cmake_minimum_required(VERSION 3.10)
project(ThreadPoolDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(tp_main example.cpp)
target_link_libraries(tp_main PRIVATE Threads::Threads)