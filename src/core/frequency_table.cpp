#include "huffman/core/frequency_table.hpp"

namespace huffman::core
{
    void FrequencyTable::accumulate(std::span<const Byte> /*data*/) noexcept
    {
    }

    std::uint64_t FrequencyTable::totalSymbols() const noexcept
    {
        return 1; // TODO(TDD): stub para o red
    }

    std::size_t FrequencyTable::distinctSymbols() const noexcept
    {
        return 1; // TODO(TDD): stub para o red
    }
}
