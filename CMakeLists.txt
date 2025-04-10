cmake_minimum_required(VERSION 3.10)
project(cbuf C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(cbuf_lib STATIC
    cbuf.c
)

enable_testing()

find_package(Threads REQUIRED)

add_subdirectory(test)