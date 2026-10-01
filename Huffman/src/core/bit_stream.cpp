#include "huffman/core/bit_stream.hpp"

namespace huffman::core
{
    void BitWriter::writeBit(int bit)
    {
        currentByte = static_cast<Byte>((currentByte << 1) | (bit & 1));
        if (++bitCount == 8)
        {
            out_.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
    }

    void BitWriter::writeBits(std::uint32_t value, int count)
    {
        for (int i = count - 1; i >= 0; --i)
        {
            writeBit(static_cast<int>((value >> i) & 1u));
        }
    }

    void BitWriter::flush()
    {
        if (bitCount > 0)
        {
            currentByte = static_cast<Byte>(currentByte << (8 - bitCount));
            out_.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
    }

    std::optional<int> BitReader::readBit() noexcept
    {
        if (byteIndex >= data_.size())
        {
            return std::nullopt;
        }

        const Byte currentByte = data_[byteIndex];
        const int bit = (currentByte >> (7 - bitCount)) & 1;

        if (++bitCount == 8)
        {
            bitCount = 0;
            ++byteIndex;
        }

        return bit;
    }
}
