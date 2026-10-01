#ifndef HUFFMAN_CORE_FREQUENCY_TABLE_HPP
#define HUFFMAN_CORE_FREQUENCY_TABLE_HPP

#include <array>
#include <cstdint>
#include <span>

#include "huffman/core/types.hpp"

namespace huffman::core
{

    class FrequencyTable
    {
    public:
        FrequencyTable() = default;

        void accumulate(std::span<const Byte> data) noexcept;

        [[nodiscard]] std::uint64_t totalSymbols() const noexcept;
        [[nodiscard]] std::size_t distinctSymbols() const noexcept;
        [[nodiscard]] std::uint64_t getFrequency(Byte symbol) const noexcept
        {
            return counts_[symbol];
        }

        void add(Byte symbol, std::uint64_t count) noexcept;

    private:
        std::array<std::uint64_t, kAlphabetSize> counts_{};
    };

}

#endif
