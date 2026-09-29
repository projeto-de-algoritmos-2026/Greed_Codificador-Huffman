#include <gtest/gtest.h>

#include <string>

#include "fakes/in_memory_file_system.hpp"
#include "huffman/app/compress_use_case.hpp"
#include "huffman/app/decompress_use_case.hpp"

using huffman::app::CompressUseCase;
using huffman::app::DecompressUseCase;
using huffman::core::ByteBuffer;
using huffman::testing::InMemoryFileSystem;

namespace
{
}
