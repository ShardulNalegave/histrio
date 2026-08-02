cmake_minimum_required(VERSION 3.15)
project(histrio_mailbox VERSION 0.1 LANGUAGES C)

set(HISTRIO_MAILBOX_SOURCES
    src/mailbox.c)

add_library(histrio_mailbox ${HISTRIO_MAILBOX_SOURCES})
target_link_libraries(histrio_coroutine PRIVATE histrio_common)

target_include_directories(
    histrio_mailbox
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
