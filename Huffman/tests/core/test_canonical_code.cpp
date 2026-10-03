#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "huffman/core/bit_stream.hpp"
#include "huffman/core/canonical_code.hpp"

using namespace huffman::core;

namespace
{
    CodeLengths GenerateCodeLengths()
    {
        CodeLengths code_lengths{};

        code_lengths['F'] = 1;
        code_lengths['E'] = 2;
        code_lengths['A'] = 4;
        code_lengths['B'] = 4;
        code_lengths['C'] = 4;
        code_lengths['D'] = 4;

        return code_lengths;
    }
}

TEST(CanonicalCode, GeneratesExpectedCanonicalBits)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(GenerateCodeLengths());

    EXPECT_EQ(canonical_code.codeFor('F').bits, 0u);
    EXPECT_EQ(canonical_code.codeFor('F').length, 1);

    EXPECT_EQ(canonical_code.codeFor('E').bits, 2u);
    EXPECT_EQ(canonical_code.codeFor('E').length, 2);

    EXPECT_EQ(canonical_code.codeFor('A').bits, 12u);
    EXPECT_EQ(canonical_code.codeFor('A').length, 4);

    EXPECT_EQ(canonical_code.codeFor('B').bits, 13u);
    EXPECT_EQ(canonical_code.codeFor('B').length, 4);

    EXPECT_EQ(canonical_code.codeFor('C').bits, 14u);
    EXPECT_EQ(canonical_code.codeFor('C').length, 4);

    EXPECT_EQ(canonical_code.codeFor('D').bits, 15u);
    EXPECT_EQ(canonical_code.codeFor('D').length, 4);
}

TEST(CanonicalCode, AbsentSymbolHasZeroLength)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(GenerateCodeLengths());
    EXPECT_EQ(canonical_code.codeFor('Z').length, 0);
}

TEST(CanonicalCode, IsDeterministic)
{
    const CanonicalCode a = CanonicalCode::fromCodeLengths(GenerateCodeLengths());
    const CanonicalCode b = CanonicalCode::fromCodeLengths(GenerateCodeLengths());

    for (int s = 0; s < 256; ++s)
    {
        EXPECT_EQ(a.codeFor(static_cast<Byte>(s)).bits,
                  b.codeFor(static_cast<Byte>(s)).bits);
        EXPECT_EQ(a.codeFor(static_cast<Byte>(s)).length,
                  b.codeFor(static_cast<Byte>(s)).length);
    }
}

TEST(CanonicalCode, DecodesEachSymbolFromOwnCode)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(GenerateCodeLengths());

    for (Byte sym : {
             Byte{'A'},
             Byte{'B'},
             Byte{'C'},
             Byte{'D'},
             Byte{'E'},
             Byte{'F'}})
    {
        const Code c = canonical_code.codeFor(sym);
        int pos = c.length;

        auto readBit = [&]() -> std::optional<int>
        {
            if (pos <= 0)
                return std::nullopt;
            --pos;
            return static_cast<int>((c.bits >> pos) & 1u);
        };

        const std::optional<Byte> dec = canonical_code.decodeSymbol(readBit);

        ASSERT_TRUE(dec.has_value());
        EXPECT_EQ(*dec, sym);
    }
}

TEST(CanonicalCode, DecodesConcatenatedStream)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(GenerateCodeLengths());

    ByteBuffer buffer;
    {
        BitWriter writer(buffer);
        writer.writeBits(canonical_code.codeFor('F').bits, canonical_code.codeFor('F').length);
        writer.writeBits(canonical_code.codeFor('E').bits, canonical_code.codeFor('E').length);
        writer.writeBits(canonical_code.codeFor('D').bits, canonical_code.codeFor('D').length);
        writer.flush();
    }

    BitReader reader(buffer);

    auto next = [&]()
    {
        return reader.readBit();
    };

    EXPECT_EQ(canonical_code.decodeSymbol(next), std::optional<Byte>{'F'});
    EXPECT_EQ(canonical_code.decodeSymbol(next), std::optional<Byte>{'E'});
    EXPECT_EQ(canonical_code.decodeSymbol(next), std::optional<Byte>{'D'});
}

TEST(CanonicalCode, InsufficientBitsReturnsEmpty)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(GenerateCodeLengths());

    bool first = true;
    auto readBit = [&]() -> std::optional<int>
    {
        if (first)
        {
            first = false;
            return 1;
        }
        return std::nullopt;
    };

    EXPECT_FALSE(canonical_code.decodeSymbol(readBit).has_value());
}

namespace
{
    bool isPrefixOf(const Code &a, const Code &b)
    {
        if (a.length > b.length)
            return false;
        return (b.bits >> (b.length - a.length)) == a.bits;
    }
}

TEST(CanonicalCode, CodesArePrefixFree)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(GenerateCodeLengths());
    const Byte symbols[] = {'A', 'B', 'C', 'D', 'E', 'F'};

    for (Byte a : symbols)
        for (Byte b : symbols)
            if (a != b)
                EXPECT_FALSE(isPrefixOf(canonical_code.codeFor(a), canonical_code.codeFor(b)))
                    << a << " é prefixo de " << b;
}

TEST(CanonicalCode, SameLengthCodesAreConsecutiveInSymbolOrder)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(GenerateCodeLengths());
    EXPECT_EQ(canonical_code.codeFor('B').bits, canonical_code.codeFor('A').bits + 1);
    EXPECT_EQ(canonical_code.codeFor('C').bits, canonical_code.codeFor('B').bits + 1);
    EXPECT_EQ(canonical_code.codeFor('D').bits, canonical_code.codeFor('C').bits + 1);
}

TEST(CanonicalCode, KeepsOriginalCodeLengths)
{
    const CodeLengths lengths = GenerateCodeLengths();
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(lengths);
    EXPECT_EQ(canonical_code.codeLengths(), lengths);
}

TEST(CanonicalCode, EmptyLengthsDecodeNothing)
{
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(CodeLengths{});
    auto readBit = []() -> std::optional<int>
    { return 0; };
    EXPECT_FALSE(canonical_code.decodeSymbol(readBit).has_value());
}

TEST(CanonicalCode, AllEightBitCodesMapToIdentity)
{
    CodeLengths lengths{};
    lengths.fill(8);
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(lengths);
    for (int s = 0; s < 256; ++s)
    {
        EXPECT_EQ(canonical_code.codeFor(static_cast<Byte>(s)).bits, static_cast<std::uint32_t>(s));
        EXPECT_EQ(canonical_code.codeFor(static_cast<Byte>(s)).length, 8);
    }
}

TEST(CanonicalCode, RoundTripThroughBitStreamForEverySymbol)
{
    CodeLengths lengths{};
    lengths['a'] = 1;
    lengths['b'] = 2;
    lengths['c'] = 3;
    lengths['d'] = 3;
    const CanonicalCode canonical_code = CanonicalCode::fromCodeLengths(lengths);
    const std::string message = "abacabadcbad";

    ByteBuffer buffer;
    {
        BitWriter writer(buffer);
        for (char ch : message)
        {
            const Code c = canonical_code.codeFor(static_cast<Byte>(ch));
            writer.writeBits(c.bits, c.length);
        }
        writer.flush();
    }

    BitReader reader(buffer);
    auto next = [&]()
    { return reader.readBit(); };
    for (char ch : message)
        EXPECT_EQ(canonical_code.decodeSymbol(next), std::optional<Byte>{static_cast<Byte>(ch)});
}
