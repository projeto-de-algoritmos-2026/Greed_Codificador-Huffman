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
    Huffman/src/core/frequency_table.cpp
    Huffman/src/core/bit_stream.cpp
    Huffman/src/core/huffman_tree.cpp
    Huffman/src/core/canonical_code.cpp
    Huffman/src/core/encoder.cpp
    Huffman/src/core/decoder.cpp
    Huffman/src/ports/file_system.cpp
    Huffman/src/app/compress_use_case.cpp
    Huffman/src/app/decompress_use_case.cpp
    Huffman/src/archive/tar_packer.cpp
    Huffman/src/infra/std_file_system.cpp
    Huffman/src/cli/args.cpp
)

target_include_directories(
    huffman_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Huffman/include
)

target_compile_options(
    huffman_lib
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

# === Executável CLI ===

add_executable(
    huffman
    apps/cli/main.cpp
)

target_link_libraries(
    huffman
    PRIVATE
        huffman_lib
)

target_compile_options(
    huffman
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
    Huffman/tests/core/test_frequency_table.cpp
    Huffman/tests/core/test_bit_stream.cpp
    Huffman/tests/core/test_huffman_tree.cpp
    Huffman/tests/core/test_canonical_code.cpp
    Huffman/tests/core/test_encoder_decoder.cpp
    Huffman/tests/ports/test_in_memory_file_system.cpp
    Huffman/tests/app/test_use_cases.cpp
    Huffman/tests/archive/test_tar_packer.cpp
    Huffman/tests/app/test_directory_use_cases.cpp
    Huffman/tests/infra/test_std_file_system.cpp
    Huffman/tests/cli/test_cli_args.cpp
    Huffman/tests/cli/test_e2e.cpp
)

target_include_directories(
    huffman_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Huffman/tests
)

target_link_libraries(
    huffman_tests
    PRIVATE
        huffman_lib
        GTest::gtest_main
)

target_compile_definitions(
    huffman_tests
    PRIVATE
        HUFFMAN_BIN="$<TARGET_FILE:huffman>"
)

add_dependencies(huffman_tests huffman)

include(GoogleTest)

gtest_discover_tests(huffman_tests)
