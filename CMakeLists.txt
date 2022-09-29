cmake_minimum_required(VERSION 3.3)
project(jimbot)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build/debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

file(GLOB_RECURSE SOURCES src/*.c)

add_executable(jimbot ${SOURCES})
target_link_libraries(jimbot sqlite3)
target_link_libraries(jimbot discord)
target_link_libraries(jimbot curl)

target_include_directories(jimbot PRIVATE src)
