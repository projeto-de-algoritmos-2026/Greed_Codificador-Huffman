#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "huffman/archive/tar_packer.hpp"

using huffman::archive::PackedEntry;
using huffman::archive::TarPacker;
using huffman::core::ByteBuffer;
using namespace std::string_literals;

namespace
{
    ByteBuffer fromStr(const std::string &str)
    {
        return ByteBuffer(str.begin(), str.end());
    }

    ByteBuffer header(std::uint32_t count)
    {
        ByteBuffer blob = {'H', 'T', 'A', 'R'};
        for (int i = 0; i < 4; ++i)
            blob.push_back(static_cast<std::uint8_t>((count >> (i * 8)) & 0xFF));
        return blob;
    }
}

TEST(TarPacker, RoundTripPreservesPathsAndContents)
{
    std::vector<PackedEntry> entries = {
        {"a.txt",
         false,
         fromStr("content A")},
        {"sub/b.bin",
         false,
         fromStr("\x00\x01\x02\xFF content B"s)},
        {"sub/empty",
         true,
         {}},
    };

    const ByteBuffer blob = TarPacker::pack(entries);
    const std::vector<PackedEntry> output = TarPacker::unpack(blob);

    ASSERT_EQ(output.size(), entries.size());
    for (std::size_t i = 0; i < entries.size(); ++i)
    {
        EXPECT_EQ(output[i].path, entries[i].path);
        EXPECT_EQ(output[i].isDirectory, entries[i].isDirectory);
        EXPECT_EQ(output[i].content, entries[i].content);
    }
}

TEST(TarPacker, RoundTripPreservesBinaryContentWithNullBytes)
{
    const ByteBuffer binary = fromStr("\x00\x01\x02\xFF content B"s);
    ASSERT_EQ(binary.size(), 14u);

    const std::vector<PackedEntry> output = TarPacker::unpack(TarPacker::pack({{"b.bin", false, binary}}));

    ASSERT_EQ(output.size(), 1u);
    EXPECT_EQ(output[0].content, binary);
}

TEST(TarPacker, UnpackTruncatedBlobThrows)
{
    ByteBuffer blob = TarPacker::pack({{"a.txt", false, fromStr("content A")}});
    blob.resize(blob.size() - 3);

    EXPECT_THROW((void)TarPacker::unpack(blob), std::runtime_error);
}

TEST(TarPacker, UnpackHugeContentLengthThrowsInsteadOfOverflowing)
{
    ByteBuffer blob = header(1);
    blob.push_back(0);
    blob.push_back(1);
    blob.push_back(0);
    blob.push_back('x');
    for (int i = 0; i < 8; ++i)
        blob.push_back(0xFF);

    EXPECT_THROW((void)TarPacker::unpack(blob), std::runtime_error);
}

TEST(TarPacker, UnpackEntryCountLargerThanBlobThrows)
{
    const ByteBuffer blob = header(0xFFFFFFFFu);

    EXPECT_THROW((void)TarPacker::unpack(blob), std::runtime_error);
}

TEST(TarPacker, PackPathLongerThanU16Throws)
{
    const std::string longPath(70000, 'a');

    EXPECT_THROW((void)TarPacker::pack({{longPath, false, {}}}), std::length_error);
}

TEST(TarPacker, UnpackTrailingBytesThrows)
{
    ByteBuffer blob = TarPacker::pack({{"a.txt", false, fromStr("content A")}});
    blob.push_back(0x42);

    EXPECT_THROW((void)TarPacker::unpack(blob), std::runtime_error);
}

TEST(TarPacker, PackDirectoryWithContentThrows)
{
    EXPECT_THROW((void)TarPacker::pack({{"dir", true, fromStr("oops")}}), std::invalid_argument);
}

TEST(TarPacker, UnpackDirectoryWithContentThrows)
{
    ByteBuffer blob = header(1);
    blob.push_back(1);
    blob.push_back(1);
    blob.push_back(0);
    blob.push_back('d');
    blob.push_back(1);
    for (int i = 0; i < 7; ++i)
        blob.push_back(0);
    blob.push_back('x');

    EXPECT_THROW((void)TarPacker::unpack(blob), std::runtime_error);
}

TEST(TarPacker, EmptyListRoundTrip)
{
    const ByteBuffer blob = TarPacker::pack({});
    EXPECT_TRUE(TarPacker::unpack(blob).empty());
}

TEST(TarPacker, RoundTripPreservesEmptyDirectory)
{
    std::vector<PackedEntry> entries = {{"folder/empty", true, {}}};
    const auto output = TarPacker::unpack(TarPacker::pack(entries));
    ASSERT_EQ(output.size(), 1u);
    EXPECT_TRUE(output[0].isDirectory);
    EXPECT_TRUE(output[0].content.empty());
}

TEST(TarPacker, RoundTripPreservesAllByteValues)
{
    ByteBuffer allBytes;
    for (int i = 0; i < 256; ++i)
        allBytes.push_back(static_cast<huffman::core::Byte>(i));
    std::vector<PackedEntry> entries = {{"raw.dat", false, allBytes}};

    const auto output = TarPacker::unpack(TarPacker::pack(entries));
    ASSERT_EQ(output.size(), 1u);
    EXPECT_EQ(output[0].content, allBytes);
}

TEST(TarPacker, UnpackInvalidMagicThrows)
{
    ByteBuffer garbage = fromStr("garbage data with invalid magic header");
    EXPECT_THROW((void)TarPacker::unpack(garbage), std::runtime_error);
}

TEST(TarPacker, UnpackBlobShorterThanMagicThrows)
{
    ByteBuffer shortBlob = {'H', 'T'};
    EXPECT_THROW((void)TarPacker::unpack(shortBlob), std::runtime_error);
}
