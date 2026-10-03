#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "fakes/in_memory_file_system.hpp"
#include "huffman/app/compress_use_case.hpp"
#include "huffman/app/container_format.hpp"
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

TEST(DirectoryUseCases, RoundTripNestedTree)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/README.md", fromStr("# Projeto\n"));
    fs.writeFile("proj/src/main.cpp", fromStr("int main() { return 0; }\n"));
    fs.writeFile("proj/src/core/util.h", fromStr("#pragma once\n"));
    fs.writeFile("proj/data/table.csv", fromStr("a,b\n1,2\n"));

    CompressUseCase{fs}.execute("proj", "proj.huff");
    DecompressUseCase{fs}.execute("proj.huff", "out");

    EXPECT_EQ(fs.readFile("out/README.md"), fs.readFile("proj/README.md"));
    EXPECT_EQ(fs.readFile("out/src/main.cpp"), fs.readFile("proj/src/main.cpp"));
    EXPECT_EQ(fs.readFile("out/src/core/util.h"), fs.readFile("proj/src/core/util.h"));
    EXPECT_EQ(fs.readFile("out/data/table.csv"), fs.readFile("proj/data/table.csv"));
}

TEST(DirectoryUseCases, PreservesEmptySubdirectory)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/a.txt", fromStr("a"));
    fs.createDirectories("proj/empty/nested");

    CompressUseCase{fs}.execute("proj", "proj.huff");
    DecompressUseCase{fs}.execute("proj.huff", "out");

    EXPECT_TRUE(fs.isDirectory("out/empty"));
    EXPECT_TRUE(fs.isDirectory("out/empty/nested"));
}

TEST(DirectoryUseCases, PreservesEmptyFileInsideDirectory)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/empty.txt", ByteBuffer{});
    fs.writeFile("proj/full.txt", fromStr("content"));

    CompressUseCase{fs}.execute("proj", "proj.huff");
    DecompressUseCase{fs}.execute("proj.huff", "out");

    ASSERT_TRUE(fs.exists("out/empty.txt"));
    EXPECT_TRUE(fs.readFile("out/empty.txt").empty());
    EXPECT_EQ(fs.readFile("out/full.txt"), fromStr("content"));
}

TEST(DirectoryUseCases, EmptyRootDirectoryRoundTrip)
{
    InMemoryFileSystem fs;
    fs.createDirectories("proj");

    CompressUseCase{fs}.execute("proj", "proj.huff");
    DecompressUseCase{fs}.execute("proj.huff", "out");

    EXPECT_TRUE(fs.isDirectory("out"));
    EXPECT_TRUE(fs.listRecursive("out").empty());
}

TEST(DirectoryUseCases, PreservesBinaryFiles)
{
    InMemoryFileSystem fs;
    ByteBuffer binary;
    for (int i = 0; i < 256; ++i)
        binary.push_back(static_cast<huffman::core::Byte>(i));
    fs.writeFile("proj/img/raw.bin", binary);

    CompressUseCase{fs}.execute("proj", "proj.huff");
    DecompressUseCase{fs}.execute("proj.huff", "out");

    EXPECT_EQ(fs.readFile("out/img/raw.bin"), binary);
}

TEST(DirectoryUseCases, RestoresSameListingAsOriginal)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/b.txt", fromStr("b"));
    fs.writeFile("proj/a/x.txt", fromStr("x"));
    fs.createDirectories("proj/c");

    CompressUseCase{fs}.execute("proj", "proj.huff");
    DecompressUseCase{fs}.execute("proj.huff", "out");

    const auto original = fs.listRecursive("proj");
    const auto restored = fs.listRecursive("out");
    ASSERT_EQ(restored.size(), original.size());
    for (std::size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(restored[i].relativePath, original[i].relativePath);
        EXPECT_EQ(restored[i].isDirectory, original[i].isDirectory);
    }
}

TEST(DirectoryUseCases, DirectoryContainerStartsWithDirectoryKind)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/a.txt", fromStr("a"));

    CompressUseCase{fs}.execute("proj", "proj.huff");

    const ByteBuffer container = fs.readFile("proj.huff");
    ASSERT_FALSE(container.empty());
    EXPECT_EQ(container.front(), static_cast<huffman::core::Byte>(huffman::app::Kind::Directory));
}

TEST(DirectoryUseCases, DoesNotModifyOriginalTree)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/a.txt", fromStr("original"));

    CompressUseCase{fs}.execute("proj", "proj.huff");
    DecompressUseCase{fs}.execute("proj.huff", "out");

    EXPECT_EQ(fs.readFile("proj/a.txt"), fromStr("original"));
}
