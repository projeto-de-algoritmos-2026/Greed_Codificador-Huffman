#ifndef HUFFMAN_CORE_CANONICAL_CODE_HPP
#define HUFFMAN_CORE_CANONICAL_CODE_HPP

#include <array>
#include <cstdint>
#include <optional>

#include "huffman/core/types.hpp"

namespace huffman::core
{
    struct Code
    {
        std::uint32_t bits = 0;
        CodeLength length = 0;
    };

    class CanonicalCode
    {
    private:
        CodeLengths lengths_{};
        std::array<Code, kAlphabetSize> codes_{};

        CodeLength maxLength_ = 0;
        std::array<std::uint32_t, kMaxUsableLength + 1> countByLength_{};
        std::array<std::uint32_t, kMaxUsableLength + 1> firstCode_{};
        std::array<std::uint32_t, kMaxUsableLength + 1> firstIndex_{};
        std::array<Byte, kAlphabetSize> sortedSymbols_{};

    public:
        [[nodiscard]] static CanonicalCode fromCodeLengths(const CodeLengths &lengths);
        [[nodiscard]] Code codeFor(Byte symbol) const noexcept
        {
            return codes_[symbol];
        };
        [[nodiscard]] const CodeLengths &codeLengths() const noexcept
        {
            return lengths_;
        };

        template <typename ReadBitFn>
        [[nodiscard]] std::optional<Byte> decodeSymbol(ReadBitFn &&readBit) const
        {
            std::uint32_t code = 0;
            for (CodeLength length = 1; length <= maxLength_; ++length)
            {
                std::uint32_t code = 0;

                for (CodeLength len = 1; len <= maxLength_; ++len)
                {
                    const std::optional<int> bit = readBit();

                    if (!bit)
                    {
                        return std::nullopt;
                    }

                    code = (code << 1) | static_cast<std::uint32_t>(*bit);
                    const std::uint32_t count = countByLength_[len];

                    if (count != 0)
                    {
                        const std::uint32_t first = firstCode_[len];

                        if (code >= first && code < first + count)
                        {
                            const std::uint32_t idx = firstIndex_[len] + (code - first);
                            return sortedSymbols_[idx];
                        }
                    }
                }
            }
            return std::nullopt;
        };
    };
}
#endif
