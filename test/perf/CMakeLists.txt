set(PERF_TESTS
    test_timeout
)

foreach(test ${PERF_TESTS})
    add_executable(${test} ${test}.c)
    target_link_libraries(${test} PRIVATE
        cbuf_lib
        test_utils
        Threads::Threads
    )
    add_test(NAME ${test} COMMAND ${test})
endforeach()