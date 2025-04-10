add_library(test_utils INTERFACE)
target_include_directories(test_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(unit)