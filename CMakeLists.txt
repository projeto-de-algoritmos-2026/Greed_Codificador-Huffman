cmake_minimum_required(VERSION 3.20)

project(
    huffman
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# === Biblioteca do projeto ===

add_library(
    huffman_lib
    src/core/frequency_table.cpp
    src/core/bit_stream.cpp
)

target_include_directories(
    huffman_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(
    huffman_lib
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

# === GoogleTest ===

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# === Testes ===

enable_testing()

add_executable(
    huffman_tests
    tests/test_frequency_table.cpp
    tests/test_bit_stream.cpp
)

target_link_libraries(
    huffman_tests
    PRIVATE
        huffman_lib
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(huffman_tests)
