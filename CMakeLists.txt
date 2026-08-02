cmake_minimum_required(VERSION 3.15)
project(histrio VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(CTest)

add_subdirectory(common)
add_subdirectory(telemetry)
add_subdirectory(coroutine)
add_subdirectory(scheduler)
add_subdirectory(mailbox)