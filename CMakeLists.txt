cmake_minimum_required(VERSION 3.1)

project(threadpool)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall)

link_libraries(pthread)

add_executable(example example.cpp)
