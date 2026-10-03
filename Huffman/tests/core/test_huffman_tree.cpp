#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

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

namespace
{
    // Custo ótimo calculado de forma independente: soma dos pesos de todas as fusões.
    std::uint64_t optimalCost(std::vector<std::uint64_t> weights)
    {
        std::uint64_t cost = 0;
        std::sort(weights.begin(), weights.end());
        while (weights.size() > 1)
        {
            const std::uint64_t merged = weights[0] + weights[1];
            cost += merged;
            weights.erase(weights.begin(), weights.begin() + 2);
            weights.insert(std::upper_bound(weights.begin(), weights.end(), merged), merged);
        }
        return cost;
    }

    std::uint64_t weightedLength(const FrequencyTable &table, const CodeLengths &lengths)
    {
        std::uint64_t cost = 0;
        for (std::size_t s = 0; s < kAlphabetSize; ++s)
            cost += table.getFrequency(static_cast<Byte>(s)) * lengths[s];
        return cost;
    }
}

TEST(HuffmanTree, KraftSumIsExactlyOneForTwoOrMoreSymbols)
{
    FrequencyTable table;
    table.accumulate(ByteBuffer{'a', 'a', 'a', 'b', 'b', 'c', 'd', 'e', 'e', 'e', 'e'});
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_DOUBLE_EQ(kraftSum(L), 1.0);
}

TEST(HuffmanTree, OnlyPresentSymbolsGetCodes)
{
    FrequencyTable table;
    table.accumulate(ByteBuffer{'x', 'y', 'y', 'z', 'z', 'z'});
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_EQ(countPresent(L), 3u);
    EXPECT_EQ(L['a'], 0);
}

TEST(HuffmanTree, UniformFrequenciesGiveBalancedTree)
{
    FrequencyTable table;
    for (int s = 0; s < 8; ++s)
        table.add(static_cast<Byte>(s), 10);
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    for (int s = 0; s < 8; ++s)
        EXPECT_EQ(L[static_cast<std::size_t>(s)], 3);
}

TEST(HuffmanTree, AllByteValuesEquallyFrequentGiveEightBits)
{
    FrequencyTable table;
    for (int s = 0; s < 256; ++s)
        table.add(static_cast<Byte>(s), 1);
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    for (const CodeLength len : L)
        EXPECT_EQ(len, 8);
}

TEST(HuffmanTree, ClassicTextbookExample)
{
    // Exemplo do CLRS: f:5 e:9 c:12 b:13 d:16 a:45
    FrequencyTable table;
    table.add('a', 45);
    table.add('b', 13);
    table.add('c', 12);
    table.add('d', 16);
    table.add('e', 9);
    table.add('f', 5);
    const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
    EXPECT_EQ(L['a'], 1);
    EXPECT_EQ(L['b'], 3);
    EXPECT_EQ(L['c'], 3);
    EXPECT_EQ(L['d'], 3);
    EXPECT_EQ(L['e'], 4);
    EXPECT_EQ(L['f'], 4);
    EXPECT_EQ(weightedLength(table, L), 224u);
}

TEST(HuffmanTree, IsDeterministicWithTies)
{
    FrequencyTable table;
    for (int s = 0; s < 20; ++s)
        table.add(static_cast<Byte>('a' + s), 7);
    EXPECT_EQ(HuffmanTree::buildHuffmanCodeLengths(table),
              HuffmanTree::buildHuffmanCodeLengths(table));
}

TEST(HuffmanTree, CostIsOptimalOnRandomTables)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> symbolCount(2, 256);
    std::uniform_int_distribution<std::uint64_t> weight(1, 10000);

    for (int trial = 0; trial < 50; ++trial)
    {
        FrequencyTable table;
        std::vector<std::uint64_t> weights;
        const int n = symbolCount(rng);
        for (int s = 0; s < n; ++s)
        {
            const std::uint64_t w = weight(rng);
            table.add(static_cast<Byte>(s), w);
            weights.push_back(w);
        }
        const CodeLengths L = HuffmanTree::buildHuffmanCodeLengths(table);
        EXPECT_EQ(weightedLength(table, L), optimalCost(weights)) << "trial " << trial;
    }
}
