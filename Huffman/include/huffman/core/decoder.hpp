#ifndef HUFFMAN_CORE_DECODER_HPP
#define HUFFMAN_CORE_DECODER_HPP

#include <span>
#include <stdexcept>

#include "huffman/core/types.hpp"

namespace huffman::core
{
    class CorruptDataError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    class HuffmanDecoder
    {
    public:
        [[nodiscard]] static ByteBuffer decode(std::span<const Byte> input);
    };

}

#endif
