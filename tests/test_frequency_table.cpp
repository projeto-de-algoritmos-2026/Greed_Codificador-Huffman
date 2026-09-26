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

// TEST(_, _) {}
