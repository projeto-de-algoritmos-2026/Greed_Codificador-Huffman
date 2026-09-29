#include <gtest/gtest.h>

#include <optional>

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
