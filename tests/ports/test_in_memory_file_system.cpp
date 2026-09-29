#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "fakes/in_memory_file_system.hpp"

using huffman::core::ByteBuffer;
using huffman::testing::InMemoryFileSystem;

TEST(InMemoryFileSystem, WrittenFileCanBeRead)
{
    InMemoryFileSystem fs;
    const ByteBuffer data = {'o', 'i'};
    fs.writeFile("proj/msg.txt", data);
    EXPECT_TRUE(fs.exists("proj/msg.txt"));
    EXPECT_EQ(fs.readFile("proj/msg.txt"), data);
}

TEST(InMemoryFileSystem, WriteFileCreatesParentDirectories)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/sub/a.bin", ByteBuffer{1, 2, 3});
    EXPECT_TRUE(fs.isDirectory("proj"));
    EXPECT_TRUE(fs.isDirectory("proj/sub"));
    EXPECT_FALSE(fs.isDirectory("proj/sub/a.bin"));
}

TEST(InMemoryFileSystem, ReadingNonexistentFileThrows)
{
    InMemoryFileSystem fs;
    EXPECT_THROW(fs.readFile("does/not_exist.txt"), std::runtime_error);
}

TEST(InMemoryFileSystem, ListRecursiveIsRelativeAndSorted)
{
    InMemoryFileSystem fs;
    fs.writeFile("proj/b.txt", ByteBuffer{'b'});
    fs.writeFile("proj/sub/a.txt", ByteBuffer{'a'});

    std::vector<std::string> nomes;
    for (const auto &e : fs.listRecursive("proj"))
        nomes.push_back(e.relativePath);

    EXPECT_EQ(nomes,
              (std::vector<std::string>{"b.txt", "sub", "sub/a.txt"}));
}

TEST(InMemoryFileSystem, EmptyDirectoryAppearsInListing)
{
    InMemoryFileSystem fs;
    fs.createDirectories("proj/empty");

    const auto entries = fs.listRecursive("proj");
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].relativePath, "empty");
    EXPECT_TRUE(entries[0].isDirectory);
}
