cmake_minimum_required(VERSION 3.15)
project(histrio_coroutines VERSION 0.1 LANGUAGES C)

set(HISTRIO_COROUTINES_SOURCES
    src/coroutines.c)

add_library(histrio_coroutines ${HISTRIO_COROUTINES_SOURCES})

target_include_directories(
    histrio_coroutines
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/include
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)
