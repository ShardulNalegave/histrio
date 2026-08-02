cmake_minimum_required(VERSION 3.15)
project(histrio_common VERSION 0.1 LANGUAGES C)

set(HISTRIO_COMMON_SOURCES
    src/common.c)

add_library(histrio_common ${HISTRIO_COMMON_SOURCES})

target_include_directories(
    histrio_common
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
