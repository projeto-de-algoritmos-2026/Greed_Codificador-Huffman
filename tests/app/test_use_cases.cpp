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
    ByteBuffer fromStr(const std::string &str)
    {
        return ByteBuffer(str.begin(), str.end());
    }
}

TEST(CompressUseCase, WritesOutputFile)
{
    InMemoryFileSystem fs;
    fs.writeFile("in.txt", fromStr("Lorem ipsum dolor sit amet, consectetur adipiscing elit. "));

    CompressUseCase{fs}.execute("in.txt", "out.huff");

    EXPECT_TRUE(fs.exists("out.huff"));
    EXPECT_FALSE(fs.readFile("out.huff").empty());
}

TEST(CompressUseCase, NonexistentInputThrows)
{
    InMemoryFileSystem fs;
    EXPECT_THROW(CompressUseCase{fs}.execute("not_exist.txt", "x.huff"),
                 std::runtime_error);
}

TEST(DecompressUseCase, NonexistentInputThrows)
{
    InMemoryFileSystem fs;
    EXPECT_THROW(DecompressUseCase{fs}.execute("not_exist.huff", "x.out"),
                 std::runtime_error);
}

TEST(UseCases, RoundTripSingleFile)
{
    InMemoryFileSystem fs;
    const ByteBuffer original =
        fromStr("Lorem ipsum dolor sit amet, consectetur adipiscing elit. ");
    fs.writeFile("doc.txt", original);

    CompressUseCase{fs}.execute("doc.txt", "doc.huff");
    DecompressUseCase{fs}.execute("doc.huff", "doc.out");

    EXPECT_EQ(fs.readFile("doc.out"), original);
}
