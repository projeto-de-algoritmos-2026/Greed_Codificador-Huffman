#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "fakes/in_memory_file_system.hpp"
#include "huffman/app/compress_use_case.hpp"
#include "huffman/app/decompress_use_case.hpp"

using huffman::app::CompressUseCase;
using huffman::app::DecompressUseCase;
using huffman::core::ByteBuffer;
using huffman::testing::InMemoryFileSystem;

namespace
{
    ByteBuffer fromStr(const std::string &str)
    {
        return ByteBuffer(str.begin(), str.end());
    }
}
