#include <gtest/gtest.h>

#include "huffman/core/bit_stream.hpp"

using huffman::core::BitReader;
using huffman::core::BitWriter;
using huffman::core::Byte;
using huffman::core::ByteBuffer;

// === BitWriter::writeBit ===

TEST(BitWriter, WriteBitDoesNotEmitByteBeforeEightBits)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    for (int i = 0; i < 7; ++i)
        writer.writeBit(1);
    EXPECT_TRUE(buffer.empty());
}

TEST(BitWriter, WriteBitEmitsByteMsbFirstAfterEightBits)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    for (int bit : {1, 0, 1, 1, 0, 0, 0, 1})
        writer.writeBit(bit);
    ASSERT_EQ(buffer.size(), 1u);
    EXPECT_EQ(buffer[0], Byte{0xB1});
}

// === BitWriter::writeBits ===

TEST(BitWriter, WriteBitsWritesFullByte)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    writer.writeBits(0xB1, 8);
    EXPECT_EQ(buffer, (ByteBuffer{0xB1}));
}

TEST(BitWriter, WriteBitsUsesOnlyLowCountBits)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    writer.writeBits(0b11111101, 3); // only "101" should be written
    writer.flush();
    EXPECT_EQ(buffer, (ByteBuffer{0b10100000}));
}

TEST(BitWriter, WriteBitsSpanningTwoBytes)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    writer.writeBits(0xABC, 12);
    writer.flush();
    EXPECT_EQ(buffer, (ByteBuffer{0xAB, 0xC0}));
}

// === BitWriter::flush ===

TEST(BitWriter, FlushPadsPartialByteWithZeros)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    writer.writeBit(1);
    writer.writeBit(1);
    writer.flush();
    EXPECT_EQ(buffer, (ByteBuffer{0b11000000}));
}

TEST(BitWriter, FlushWithoutPendingBitsEmitsNothing)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    writer.flush();
    EXPECT_TRUE(buffer.empty());
}

TEST(BitWriter, FlushTwiceDoesNotDuplicateByte)
{
    ByteBuffer buffer;
    BitWriter writer(buffer);
    writer.writeBit(1);
    writer.flush();
    writer.flush();
    EXPECT_EQ(buffer.size(), 1u);
}

// === BitReader::readBit ===

TEST(BitReader, ReadBitReturnsBitsMsbFirst)
{
    const ByteBuffer data = {0xB1};
    BitReader reader(data);
    for (int expected : {1, 0, 1, 1, 0, 0, 0, 1})
    {
        auto bit = reader.readBit();
        ASSERT_TRUE(bit.has_value());
        EXPECT_EQ(*bit, expected);
    }
}

TEST(BitReader, ReadBitOnEmptyDataReturnsNullopt)
{
    const ByteBuffer data;
    BitReader reader(data);
    EXPECT_FALSE(reader.readBit().has_value());
}

TEST(BitReader, ReadBitAfterEndReturnsNullopt)
{
    const ByteBuffer data = {0xFF};
    BitReader reader(data);
    for (int i = 0; i < 8; ++i)
        (void)reader.readBit();
    EXPECT_FALSE(reader.readBit().has_value());
}

TEST(BitReader, ReadBitCrossesByteBoundary)
{
    const ByteBuffer data = {0x01, 0x80};

    BitReader reader(data);
    for (int i = 0; i < 7; ++i)
        (void)reader.readBit();

    auto readBit = reader.readBit();
    ASSERT_TRUE(readBit.has_value());
    EXPECT_EQ(*readBit, 1);

    readBit = reader.readBit();
    ASSERT_TRUE(readBit.has_value());
    EXPECT_EQ(*readBit, 1);
}

// === BitReader::exhausted ===

TEST(BitReader, ExhaustedIsTrueForEmptyData)
{
    const ByteBuffer data;
    BitReader reader(data);
    EXPECT_TRUE(reader.exhausted());
}

TEST(BitReader, ExhaustedIsFalseWhileBitsRemain)
{
    const ByteBuffer data = {0x00};
    BitReader reader(data);
    for (int i = 0; i < 7; ++i)
        (void)reader.readBit();
    EXPECT_FALSE(reader.exhausted());
}

TEST(BitReader, ExhaustedIsTrueAfterReadingAllBits)
{
    const ByteBuffer data = {0x00};
    BitReader reader(data);
    for (int i = 0; i < 8; ++i)
        (void)reader.readBit();
    EXPECT_TRUE(reader.exhausted());
}

// === BitStream ===

TEST(BitStream, WriteAndReadBits)
{
    ByteBuffer buffer;

    const int bits[] = {1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0};
    {
        BitWriter writer(buffer);
        for (int bit : bits)
            writer.writeBit(bit);
        writer.flush();
    }

    BitReader reader(buffer);

    for (int bit : bits)
    {
        auto readBit = reader.readBit();
        ASSERT_TRUE(readBit.has_value());
        EXPECT_EQ(*readBit, bit);
    }
}
