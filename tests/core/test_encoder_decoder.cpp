#include <gtest/gtest.h>

#include <cstdint>
#include <random>
#include <string>

#include "huffman/core/decoder.hpp"
#include "huffman/core/encoder.hpp"

using namespace huffman::core;

namespace
{
    ByteBuffer fromStr(const std::string &str)
    {
        return ByteBuffer(str.begin(), str.end());
    }

    void roundTrip(const ByteBuffer &in)
    {
        const EncodedBlock block = HuffmanEncoder::encode(in);
        const ByteBuffer out = HuffmanDecoder::decode(block.bytes);
        ASSERT_EQ(out.size(), in.size());
        EXPECT_EQ(out, in);
    }
};

TEST(Encoder, HeaderHasCountAndLengths)
{
    const EncodedBlock block = HuffmanEncoder::encode(fromStr("aab"));
    ASSERT_GE(block.bytes.size(), 8u + 256u);

    std::uint64_t count = 0;
    for (int i = 0; i < 8; ++i)
        count |= static_cast<std::uint64_t>(block.bytes[static_cast<std::size_t>(i)]) << (i * 8);
    EXPECT_EQ(count, 3u);

    EXPECT_GT(block.bytes[8 + 'a'], 0);
    EXPECT_GT(block.bytes[8 + 'b'], 0);
    EXPECT_EQ(block.bytes[8 + 'z'], 0);
}

TEST(RoundTrip, Empty) { roundTrip(ByteBuffer{}); }
TEST(RoundTrip, SingleByte) { roundTrip(ByteBuffer{0x42}); }
TEST(RoundTrip, SingleRepeatedSymbol) { roundTrip(ByteBuffer(1000, 0xAB)); }
TEST(RoundTrip, Text) { roundTrip(fromStr("Huffman")); }

TEST(RoundTrip, AllByteValues)
{
    ByteBuffer d;
    for (int i = 0; i < 256; ++i)
        for (int k = 0; k <= i; ++k)
            d.push_back(static_cast<Byte>(i));
    roundTrip(d);
}

TEST(RoundTrip, LargeRandom)
{
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(0, 255);
    ByteBuffer d(200000);
    for (auto &b : d)
        b = static_cast<Byte>(dist(rng));
    roundTrip(d);
}

TEST(RoundTrip, SkewedInputCompresses)
{
    std::mt19937 rng(777);
    std::discrete_distribution<int> dist({90, 5, 3, 1, 1});
    ByteBuffer d(100000);
    for (auto &b : d)
        b = static_cast<Byte>(dist(rng));

    const EncodedBlock block = HuffmanEncoder::encode(d);
    roundTrip(d);
    EXPECT_LT(block.bytes.size(), d.size());
}

TEST(Decoder, DetectsTruncation)
{
    EncodedBlock block =
        HuffmanEncoder::encode(fromStr("dados suficientes para gerar muitos bits"));
    const std::size_t header = 8u + 256u;
    ASSERT_GT(block.bytes.size(), header);

    const std::size_t payload = block.bytes.size() - header;
    block.bytes.resize(header + payload / 2);

    EXPECT_THROW(static_cast<void>(HuffmanDecoder::decode(block.bytes)), CorruptDataError);
}

TEST(Decoder, IncompleteHeaderThrows)
{
    const ByteBuffer curto(10, 0);
    EXPECT_THROW(static_cast<void>(HuffmanDecoder::decode(curto)), CorruptDataError);
}
