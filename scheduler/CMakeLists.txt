cmake_minimum_required(VERSION 3.15)
project(histrio_scheduler VERSION 0.1 LANGUAGES C)

set(HISTRIO_SCHEDULER_SOURCES
    src/scheduler.c)

add_library(histrio_scheduler ${HISTRIO_SCHEDULER_SOURCES})
target_link_libraries(histrio_coroutine PRIVATE histrio_common)
target_link_libraries(histrio_scheduler PRIVATE histrio_coroutine)

target_include_directories(
    histrio_scheduler
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
