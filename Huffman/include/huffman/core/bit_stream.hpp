#ifndef HUFFMAN_CORE_BIT_STREAM_HPP
#define HUFFMAN_CORE_BIT_STREAM_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "huffman/core/types.hpp"

namespace huffman::core
{
    class BitWriter
    {
    private:
        ByteBuffer &out_;
        Byte currentByte = 0;
        int bitCount = 0;

    public:
        explicit BitWriter(ByteBuffer &out) noexcept : out_(out) {}

        void writeBit(int bit);

        void writeBits(std::uint32_t value, int count);

        void flush();
    };

    class BitReader
    {
    private:
        std::span<const Byte> data_;
        std::size_t byteIndex = 0;
        int bitCount = 0;

    public:
        explicit BitReader(std::span<const Byte> data) noexcept : data_(data) {}

        [[nodiscard]] std::optional<int> readBit() noexcept;

        [[nodiscard]] bool exhausted() const noexcept
        {
            return byteIndex >= data_.size();
        }
    };
};

#endif
