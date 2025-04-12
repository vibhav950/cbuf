cmake_minimum_required(VERSION 3.10)
project(cbuf C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -DENABLE_TSAN=OFF [default]
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)

if(ENABLE_TSAN)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    message(STATUS "Enabling TSAN")
    set(SANITIZE_FLAGS "-fsanitize=thread -g -O1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAGS}")
  else()
    message(WARNING "TSAN not available")
  endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(cbuf_lib STATIC
    cbuf.c
)

enable_testing()

find_package(Threads REQUIRED)

add_subdirectory(test)