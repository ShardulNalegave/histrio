cmake_minimum_required(VERSION 3.15)
project(histrio_coroutine VERSION 0.1 LANGUAGES C ASM)

set(HISTRIO_COROUTINE_SOURCES
    src/coroutine.c
    src/context_switch.S
    src/trampoline.S)

add_library(histrio_coroutine ${HISTRIO_COROUTINE_SOURCES})
target_link_libraries(histrio_coroutine PUBLIC histrio_common)
target_link_libraries(histrio_coroutine PRIVATE histrio_telemetry)

target_include_directories(
    histrio_coroutine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
