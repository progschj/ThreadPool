cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(ThreadPool)
set(PACKAGE_VERSION 1.0.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat-pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=2")

if (APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
endif ()

find_package(glog   REQUIRED)
find_package(gtest  REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})


find_program(clang_format "clang-format")
if(clang_format)
    file(GLOB_RECURSE all_cxx_source_files ${PROJECT_SOURCE_DIR}/*.cc ${PROJECT_SOURCE_DIR}/*.h)
    add_custom_target(clang-format COMMAND ${clang_format} -i -style=file ${all_cxx_source_files})
endif()


set(libraries       glog)
set(test_libraries  ${libraries} gtest gmock)

add_executable(example          example.cpp)
target_link_libraries(example   ${libraries})