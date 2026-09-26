#include <gtest/gtest.h>

#include "huffman/core/frequency_table.hpp"

using huffman::core::Byte;
using huffman::core::FrequencyTable;

TEST(FrequencyTable, DefaultConstructor)
{
    FrequencyTable table;
    EXPECT_EQ(table.totalSymbols(), 0u);
    EXPECT_EQ(table.distinctSymbols(), 0u);
    EXPECT_EQ(table.getFrequency(Byte{'A'}), 0u);
}

TEST(FrequencyTable, TotalSymbolsAfterAccumulate)
{
    FrequencyTable table;
    const huffman::core::ByteBuffer data = {{'A', 'B', 'A', 'C', 'B'}};
    table.accumulate(data);
    EXPECT_EQ(table.totalSymbols(), 5u);
}

TEST(FrequencyTable, DistinctSymbolsAfterAccumulate)
{
    FrequencyTable table;
    const huffman::core::ByteBuffer data = {{'A', 'B', 'A', 'C', 'B'}};
    table.accumulate(data);
    EXPECT_EQ(table.distinctSymbols(), 3u);
}

TEST(FrequencyTable, GetFrequencyAfterAccumulate)
{
    FrequencyTable table;
    const huffman::core::ByteBuffer data = {{'A', 'B', 'A', 'C', 'B'}};
    table.accumulate(data);
    EXPECT_EQ(table.getFrequency(Byte{'A'}), 2u);
    EXPECT_EQ(table.getFrequency(Byte{'B'}), 2u);
    EXPECT_EQ(table.getFrequency(Byte{'C'}), 1u);
    EXPECT_EQ(table.getFrequency(Byte{'Z'}), 0u);
}

TEST(FrequencyTable, AccumulateMultipleTimes)
{
    FrequencyTable table;
    table.accumulate(huffman::core::ByteBuffer{{'A', 'A'}});
    table.accumulate(huffman::core::ByteBuffer{{'A', 'B'}});

    EXPECT_EQ(table.getFrequency(Byte{'A'}), 3u);
    EXPECT_EQ(table.getFrequency(Byte{'B'}), 1u);
    EXPECT_EQ(table.totalSymbols(), 4u);
}

// TEST(FrequencyTable, _) {}
