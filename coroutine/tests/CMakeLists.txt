find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED IMPORTED_TARGET criterion>=2.4)

set(HISTRIO_COROUTINE_TEST_SOURCES
    support/stack_alloc.c
    test_create.c
    test_lifecycle.c
    test_context_switch.c
    test_execution.c
    test_multiple.c
    test_misuse.c)

add_executable(histrio_coroutine_tests ${HISTRIO_COROUTINE_TEST_SOURCES})
target_include_directories(histrio_coroutine_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(histrio_coroutine_tests PRIVATE histrio_coroutine histrio_telemetry PkgConfig::CRITERION)
target_link_libraries(histrio_coroutine_tests PRIVATE stdc++)
add_test(NAME histrio_coroutine_tests COMMAND histrio_coroutine_tests)

# Use AddressSanitizer for the tests suite.
# We do not enable ASan for the actual library so that they don't carry the additional overhead.
option(HISTRIO_COROUTINE_TESTS_ASAN "Also build and run the coroutine test suite under AddressSanitizer" ON)
if(HISTRIO_COROUTINE_TESTS_ASAN)
    add_library(histrio_coroutine_asan STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/coroutine.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/context_switch.S
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/trampoline.S)
    target_include_directories(
        histrio_coroutine_asan
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../src/include
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    target_link_libraries(histrio_coroutine_asan PUBLIC histrio_common PRIVATE histrio_telemetry)
    target_compile_options(histrio_coroutine_asan PUBLIC -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(histrio_coroutine_asan PUBLIC -fsanitize=address)

    add_executable(histrio_coroutine_tests_asan ${HISTRIO_COROUTINE_TEST_SOURCES} support/asan_options.c)
    target_include_directories(histrio_coroutine_tests_asan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(histrio_coroutine_tests_asan PRIVATE histrio_coroutine_asan histrio_telemetry PkgConfig::CRITERION)
    target_compile_options(histrio_coroutine_tests_asan PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(histrio_coroutine_tests_asan PRIVATE -fsanitize=address)
    target_link_libraries(histrio_coroutine_tests_asan PRIVATE stdc++)
    add_test(NAME histrio_coroutine_tests_asan COMMAND histrio_coroutine_tests_asan)
endif()
