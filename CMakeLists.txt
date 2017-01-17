cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(ThreadPool)
set(PACKAGE_VERSION 1.0.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat-pedantic")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=2")

    if (APPLE)
        set(CMAKE_MACOSX_RPATH ON)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    endif ()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors==2")
endif()


include_directories(/usr/lib)
include_directories(/usr/local/lib)
include_directories(${PROJECT_SOURCE_DIR})

find_package(Threads)

find_program(clang_format "clang-format")
if(clang_format)
    file(GLOB_RECURSE all_cxx_source_files ${PROJECT_SOURCE_DIR}/*.cc ${PROJECT_SOURCE_DIR}/*.h)
    add_custom_target(clang-format COMMAND ${clang_format} -i -style=file ${all_cxx_source_files})
endif()


set(libraries       ${CMAKE_THREAD_LIBS_INIT})
set(test_libraries  ${libraries} gtest gmock)

add_executable(example          examples/example.cpp)
target_link_libraries(example   ${libraries})

file(GLOB_RECURSE all_cxx_test_files       ${PROJECT_SOURCE_DIR}/tests/*.cc)
add_executable(test_thread_pool ${all_cxx_test_files})
target_link_libraries(test_thread_pool     gtest gtest_main gmock ${libraries})
add_test(test_thread_pool       test_thread_pool)

enable_testing()