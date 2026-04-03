cmake_minimum_required(VERSION 4.2)
project(text C)

set(CMAKE_C_STANDARD 11)

add_executable(text main.c
        algorithms/algorithms_filter.c
        algorithms/algorithms_filter.h
        algorithms/algorithms_queue.c
        algorithms/algorithms_queue.h)

target_include_directories(text PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

