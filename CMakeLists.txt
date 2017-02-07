cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(ThreadPool)
set(PACKAGE_VERSION 1.0.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat-pedantic")

    if (APPLE)
        set(CMAKE_MACOSX_RPATH ON)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    endif ()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors=2")
endif()

function(create_executable exe_path)
    set(exe_name ${exe_path})

    string(REPLACE "/" ";" exe_name_list ${exe_name})
    list(GET exe_name_list -1 exe_name)
    string(REPLACE "." ";" exe_name_list ${exe_name})
    list(GET exe_name_list 0 exe_name)

    message("-- adding executable: ${exe_name}")

    add_executable(${exe_name}         ${exe_path})
    target_link_libraries(${exe_name}  ${ARGN})
endfunction()


function(create_test    test_path)
    set(test_name ${test_path})

    string(REPLACE "/" ";" test_name_list ${test_name})
    list(GET test_name_list -1 test_name)
    string(REPLACE "." ";" test_name_list ${test_name})
    list(GET test_name_list 0 test_name)

    message("-- adding test: ${test_name}")

    add_executable(${test_name}         test/${test_path})
    target_link_libraries(${test_name}  ${ARGN})
    add_test(${test_name} ${test_name})
endfunction()

link_directories(/usr/lib)
link_directories(/usr/local/lib)
include_directories(${PROJECT_SOURCE_DIR})

find_package(Threads)


find_program(clang_format "clang-format")
if(clang_format)
    file(GLOB_RECURSE all_cxx_source_files ${PROJECT_SOURCE_DIR}/*.cc ${PROJECT_SOURCE_DIR}/*.h)
    add_custom_target(clang-format COMMAND ${clang_format} -i -style=file ${all_cxx_source_files})
endif()


set(libraries       ${CMAKE_THREAD_LIBS_INIT})

create_executable(examples/example.cc   ${libraries})


find_package(GTest)
if (GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    set(test_libraries  ${libraries} gtest gtest_main gmock pthread)

    create_test(test_thread_pool.cc    ${test_libraries})

    enable_testing()
endif ()

