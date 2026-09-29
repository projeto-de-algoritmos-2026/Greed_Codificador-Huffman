#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "huffman/core/frequency_table.hpp"
#include "huffman/core/huffman_tree.hpp"

using namespace huffman::core;

namespace
{
    double kraftSum(const CodeLengths &codeLengths)
    {
        double sum = 0.0;
        for (const auto &length : codeLengths)
        {
            if (length > 0)
            {
                sum += std::ldexp(1.0, -static_cast<int>(length));
            }
        }
        return sum;
    }

    CodeLength maxLength(const CodeLengths &codeLengths)
    {
        CodeLength maxLen = 0;
        for (CodeLength length : codeLengths)
            maxLen = std::max(maxLen, length);
        return maxLen;
    }

    std::size_t countPresent(const CodeLengths &codeLengths)
    {
        std::size_t count = 0;
        for (CodeLength length : codeLengths)
            if (length > 0)
                ++count;
        return count;
    }

}

TEST(HuffmanTree, BuildHuffmanCodeLengthsEmptyTable)
{
    FrequencyTable table;
    const CodeLengths codeLengths = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_EQ(kraftSum(codeLengths), 0.0);
    EXPECT_EQ(maxLength(codeLengths), 0);
    EXPECT_EQ(countPresent(codeLengths), 0u);
}

TEST(HuffmanTree, MostFrequentDoesNotHaveLongestCode)
{
    FrequencyTable table;
    table.accumulate(ByteBuffer{'a', 'a', 'a', 'a', 'b', 'b', 'c'});
    const CodeLengths codeLengths = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_LE(codeLengths['a'], codeLengths['b']);
    EXPECT_LE(codeLengths['b'], codeLengths['c']);
}

TEST(HuffmanTree, KnownLengthsForSimpleCase)
{
    FrequencyTable table;
    table.accumulate(ByteBuffer{'a', 'a', 'a', 'a', 'b', 'b', 'c'});
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_EQ(L['a'], 1);
    EXPECT_EQ(L['b'], 2);
    EXPECT_EQ(L['c'], 2);
}

TEST(HuffmanTree, SatisfiesKraftInequality)
{
    FrequencyTable table;
    table.accumulate(ByteBuffer{'a', 'a', 'a', 'a', 'b', 'b', 'c'});
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_LE(kraftSum(L), 1.0 + 1e-9);
}

TEST(HuffmanTree, SingleSymbolGetsOneBit)
{
    FrequencyTable table;
    table.accumulate(ByteBuffer(100, 'x'));
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_EQ(L['x'], 1);
    EXPECT_EQ(countPresent(L), 1u);
}

TEST(HuffmanTree, TwoSymbolsOneBitEach)
{
    FrequencyTable table;
    table.accumulate(ByteBuffer{'a', 'a', 'a', 'b'});
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_EQ(L['a'], 1);
    EXPECT_EQ(L['b'], 1);
}

TEST(HuffmanTree, LimitsDepthOnFibonacciInput)
{
    FrequencyTable table;
    std::uint64_t a = 1, b = 1;
    for (int s = 0; s < 40; ++s)
    {
        table.add(static_cast<Byte>(s), a);
        const std::uint64_t next = a + b;
        a = b;
        b = next;
    }
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_LE(maxLength(L), HuffmanTree::kMaxCodeLength);
    EXPECT_LE(kraftSum(L), 1.0 + 1e-9);
    EXPECT_EQ(countPresent(L), 40u);
}
