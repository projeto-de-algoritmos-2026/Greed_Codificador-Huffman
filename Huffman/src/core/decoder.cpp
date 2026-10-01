#include "huffman/core/decoder.hpp"

#include <cstdint>
#include <optional>

#include "huffman/core/bit_stream.hpp"
#include "huffman/core/canonical_code.hpp"

namespace huffman::core
{
    namespace
    {
        std::uint64_t readUint64LE(std::span<const Byte> data, std::size_t offset)
        {
            std::uint64_t value = 0;
            for (int i = 0; i < 8; ++i)
                value |= static_cast<std::uint64_t>(
                             data[offset + static_cast<std::size_t>(i)])
                         << (i * 8);
            return value;
        }

        constexpr std::size_t kHeaderFixed = 8;
        constexpr std::size_t kHeaderTotal = kHeaderFixed + kAlphabetSize;
    }

    ByteBuffer HuffmanDecoder::decode(std::span<const Byte> input)
    {
        if (input.size() < kHeaderTotal)
            throw CorruptDataError("truncated data: incomplete header");

        const std::uint64_t symbolCount = readUint64LE(input, 0);

        CodeLengths lengths{};
        for (std::size_t s = 0; s < kAlphabetSize; ++s)
            lengths[s] = input[kHeaderFixed + s];

        ByteBuffer output;
        output.reserve(static_cast<std::size_t>(symbolCount));
        if (symbolCount == 0)
            return output;

        const CanonicalCode code = CanonicalCode::fromCodeLengths(lengths);
        const std::span<const Byte> payload = input.subspan(kHeaderTotal);
        BitReader reader(payload);

        for (std::uint64_t i = 0; i < symbolCount; ++i)
        {
            const std::optional<Byte> sym =
                code.decodeSymbol([&reader]()
                                  { return reader.readBit(); });
            if (!sym)
                throw CorruptDataError("truncated data: fewer symbols than expected");
            output.push_back(*sym);
        }

        return output;
    }
}
