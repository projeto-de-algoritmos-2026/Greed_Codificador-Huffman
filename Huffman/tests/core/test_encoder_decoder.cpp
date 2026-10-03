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
        HuffmanEncoder::encode(fromStr("enough data to generate many bits"));
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

TEST(Encoder, EmptyInputProducesOnlyHeader)
{
    const EncodedBlock block = HuffmanEncoder::encode(ByteBuffer{});
    ASSERT_EQ(block.bytes.size(), 8u + 256u);
    for (const Byte b : block.bytes)
        EXPECT_EQ(b, 0);
}

TEST(Encoder, IsDeterministic)
{
    const ByteBuffer in = fromStr("the same input must always produce the same output");
    EXPECT_EQ(HuffmanEncoder::encode(in).bytes, HuffmanEncoder::encode(in).bytes);
}

TEST(Encoder, PayloadSizeMatchesSumOfCodeLengths)
{
    const ByteBuffer in = fromStr("abracadabra, abracadabra!");
    const EncodedBlock block = HuffmanEncoder::encode(in);

    std::uint64_t totalBits = 0;
    for (const Byte b : in)
        totalBits += block.bytes[8 + b];

    const std::size_t payload = block.bytes.size() - (8u + 256u);
    EXPECT_EQ(payload, (totalBits + 7) / 8);
}

TEST(Encoder, PaddingBitsAreZero)
{
    // 3 símbolos de 1 bit cada => 3 bits úteis e 5 de padding.
    const EncodedBlock block = HuffmanEncoder::encode(fromStr("aab"));
    ASSERT_EQ(block.bytes.size(), 8u + 256u + 1u);
    EXPECT_EQ(block.bytes.back() & 0x1F, 0);
}

TEST(Encoder, FrequentSymbolGetsShorterCodeThanRareOne)
{
    std::string text(500, 'e');
    text += "xyz";
    const EncodedBlock block = HuffmanEncoder::encode(fromStr(text));
    EXPECT_LT(block.bytes[8 + 'e'], block.bytes[8 + 'x']);
}

TEST(RoundTrip, TwoAlternatingSymbols) { roundTrip(fromStr("abababababababababab")); }

TEST(RoundTrip, Utf8Text)
{
    roundTrip(fromStr("Codificação de Huffman — compressão sem perdas: ç, ã, é, ü, 漢字"));
}

TEST(RoundTrip, NullBytes) { roundTrip(ByteBuffer(64, 0x00)); }

TEST(RoundTrip, FibonacciFrequenciesHitLengthLimit)
{
    ByteBuffer d;
    std::uint64_t a = 1, b = 1;
    for (int s = 0; s < 26; ++s)
    {
        d.insert(d.end(), static_cast<std::size_t>(a), static_cast<Byte>(s));
        const std::uint64_t next = a + b;
        a = b;
        b = next;
    }
    roundTrip(d);
}

TEST(RoundTrip, ManyRandomSizes)
{
    std::mt19937 rng(2026);
    std::uniform_int_distribution<int> byteDist(0, 255);
    for (std::size_t size : {1u, 2u, 3u, 7u, 8u, 9u, 255u, 256u, 257u, 4096u})
    {
        ByteBuffer d(size);
        for (auto &b : d)
            b = static_cast<Byte>(byteDist(rng) % 16);
        roundTrip(d);
    }
}

TEST(Decoder, HeaderWithSymbolsButNoCodesThrows)
{
    ByteBuffer corrupt(8u + 256u + 4u, 0);
    corrupt[0] = 5; // diz que há 5 símbolos, mas todos os comprimentos são zero
    EXPECT_THROW(static_cast<void>(HuffmanDecoder::decode(corrupt)), CorruptDataError);
}

TEST(Decoder, SymbolCountLargerThanPayloadThrows)
{
    EncodedBlock block = HuffmanEncoder::encode(fromStr("short"));
    block.bytes[0] = 0xFF; // aumenta a contagem de símbolos sem aumentar os dados
    EXPECT_THROW(static_cast<void>(HuffmanDecoder::decode(block.bytes)), CorruptDataError);
}

TEST(Decoder, HeaderOnlyWithZeroCountReturnsEmpty)
{
    const ByteBuffer headerOnly(8u + 256u, 0);
    EXPECT_TRUE(HuffmanDecoder::decode(headerOnly).empty());
}
