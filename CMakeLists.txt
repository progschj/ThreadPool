project(thread_pool)
cmake_minimum_required(VERSION 2.6)
include_directories(./)
add_definitions("-Wall -std=c++11 -g")

aux_source_directory(. SRC)
add_executable(theadpool ${SRC})
target_link_libraries(theadpool -pthread)