#include <gtest/gtest.h>

#include <string>

#include "fakes/in_memory_file_system.hpp"
#include "huffman/app/compress_use_case.hpp"
#include "huffman/app/container_format.hpp"
#include "huffman/app/decompress_use_case.hpp"
#include "huffman/core/decoder.hpp"

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

TEST(CompressUseCase, FileContainerStartsWithFileKind)
{
    InMemoryFileSystem fs;
    fs.writeFile("in.txt", fromStr("abc"));

    CompressUseCase{fs}.execute("in.txt", "out.huff");

    const ByteBuffer container = fs.readFile("out.huff");
    ASSERT_FALSE(container.empty());
    EXPECT_EQ(container.front(), static_cast<huffman::core::Byte>(huffman::app::Kind::File));
}

TEST(DecompressUseCase, EmptyCompressedFileThrows)
{
    InMemoryFileSystem fs;
    fs.writeFile("empty.huff", ByteBuffer{});

    EXPECT_THROW(DecompressUseCase{fs}.execute("empty.huff", "x.out"),
                 std::runtime_error);
}

TEST(DecompressUseCase, TruncatedCompressedFileThrows)
{
    InMemoryFileSystem fs;
    fs.writeFile("doc.txt", fromStr("a text long enough to produce several payload bytes"));
    CompressUseCase{fs}.execute("doc.txt", "doc.huff");

    ByteBuffer container = fs.readFile("doc.huff");
    container.resize(container.size() - 5);
    fs.writeFile("doc.huff", container);

    EXPECT_THROW(DecompressUseCase{fs}.execute("doc.huff", "doc.out"),
                 huffman::core::CorruptDataError);
}

TEST(UseCases, RoundTripEmptyFile)
{
    InMemoryFileSystem fs;
    fs.writeFile("empty.txt", ByteBuffer{});

    CompressUseCase{fs}.execute("empty.txt", "empty.huff");
    DecompressUseCase{fs}.execute("empty.huff", "empty.out");

    ASSERT_TRUE(fs.exists("empty.out"));
    EXPECT_TRUE(fs.readFile("empty.out").empty());
}

TEST(UseCases, RoundTripBinaryFile)
{
    InMemoryFileSystem fs;
    ByteBuffer original;
    for (int rep = 0; rep < 10; ++rep)
        for (int b = 0; b < 256; ++b)
            original.push_back(static_cast<huffman::core::Byte>(b));
    fs.writeFile("raw.bin", original);

    CompressUseCase{fs}.execute("raw.bin", "raw.huff");
    DecompressUseCase{fs}.execute("raw.huff", "raw.out");

    EXPECT_EQ(fs.readFile("raw.out"), original);
}

TEST(UseCases, DecompressWritesIntoNestedOutputPath)
{
    InMemoryFileSystem fs;
    fs.writeFile("doc.txt", fromStr("hello"));

    CompressUseCase{fs}.execute("doc.txt", "doc.huff");
    DecompressUseCase{fs}.execute("doc.huff", "restored/deep/doc.txt");

    EXPECT_EQ(fs.readFile("restored/deep/doc.txt"), fromStr("hello"));
}
