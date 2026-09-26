#include "huffman/core/frequency_table.hpp"

namespace huffman::core
{
    void FrequencyTable::accumulate(std::span<const Byte> data) noexcept
    {
        for (const Byte symbol : data)
        {
            ++counts_[symbol];
        }
    }

    std::uint64_t FrequencyTable::totalSymbols() const noexcept
    {
        std::uint64_t total = 0;
        for (const std::uint64_t count : counts_)
        {
            total += count;
        }
        return total;
    }

    std::size_t FrequencyTable::distinctSymbols() const noexcept
    {
        std::size_t distinct = 0;
        for (const std::uint64_t count : counts_)
        {
            if (count != 0)
            {
                ++distinct;
            }
        }
        return distinct;
    }
}
