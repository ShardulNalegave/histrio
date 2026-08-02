cmake_minimum_required(VERSION 3.15)
project(histrio_telemetry VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED)

set(HISTRIO_TELEMETRY_SOURCES
    src/log_shim.cc)

add_library(histrio_telemetry ${HISTRIO_TELEMETRY_SOURCES})

target_link_libraries(
    histrio_telemetry
    PUBLIC
        histrio_common
    PRIVATE
        spdlog::spdlog
)

target_include_directories(
    histrio_telemetry
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
