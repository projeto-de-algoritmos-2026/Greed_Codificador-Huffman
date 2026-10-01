#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "huffman/infra/std_file_system.hpp"

using huffman::core::ByteBuffer;
using huffman::infra::StdFileSystem;

namespace fs = std::filesystem;

class StdFileSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        root_ = fs::temp_directory_path() /
                fs::path("huff_it_" +
                         std::to_string(::testing::UnitTest::GetInstance()->random_seed()) + "_" +
                         std::to_string(reinterpret_cast<std::uintptr_t>(this)));

        fs::create_directories(root_);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(root_, ec);
    }

    fs::path root_;
    StdFileSystem sut_;
};

TEST_F(StdFileSystemTest, WrittenFileCanBeRead)
{
    const fs::path p = root_ / "a.bin";
    const ByteBuffer data = {0x00, 0x10, 0x20, 0xFF};

    sut_.writeFile(p, data);

    EXPECT_TRUE(sut_.exists(p));
    EXPECT_FALSE(sut_.isDirectory(p));
    EXPECT_EQ(sut_.readFile(p), data);
}

TEST_F(StdFileSystemTest, WriteFileCreatesParentDirectories)
{
    const fs::path p = root_ / "x" / "y" / "z.txt";

    sut_.writeFile(p, ByteBuffer{'k'});

    EXPECT_TRUE(fs::exists(p));
    EXPECT_TRUE(sut_.isDirectory(root_ / "x" / "y"));
}

TEST_F(StdFileSystemTest, ReadingNonexistentFileThrows)
{
    EXPECT_THROW((void)sut_.readFile(root_ / "does_not_exist"), std::runtime_error);
}

TEST_F(StdFileSystemTest, ListRecursiveIsRelativeAndSorted)
{
    sut_.writeFile(root_ / "b.txt", ByteBuffer{'b'});
    sut_.writeFile(root_ / "sub" / "a.txt", ByteBuffer{'a'});
    sut_.createDirectories(root_ / "empty");

    std::vector<std::string> names;

    for (const auto &e : sut_.listRecursive(root_))
        names.push_back(e.relativePath);

    EXPECT_EQ(names, (std::vector<std::string>{"b.txt", "empty", "sub", "sub/a.txt"}));
}

TEST_F(StdFileSystemTest, BinaryContentIsPreserved)
{
    ByteBuffer allBytes;

    for (int i = 0; i < 256; ++i)
        allBytes.push_back(static_cast<huffman::core::Byte>(i));

    const fs::path p = root_ / "raw.dat";

    sut_.writeFile(p, allBytes);

    EXPECT_EQ(sut_.readFile(p), allBytes);
}
