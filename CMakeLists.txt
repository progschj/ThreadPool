cmake_minimum_required(VERSION 3.0.0)
project(ThreadPool VERSION 0.1.0)
add_executable(ThreadPool example.cpp ThreadPool.h)
target_compile_features(ThreadPool PUBLIC cxx_std_11)
set_target_properties(ThreadPool PROPERTIES CXX_EXTENSIONS OFF)




