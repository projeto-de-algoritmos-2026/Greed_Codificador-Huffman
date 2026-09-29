#ifndef HUFFMAN_CORE_ENCODER_HPP
#define HUFFMAN_CORE_ENCODER_HPP

#include <span>

#include "huffman/core/types.hpp"

namespace huffman::core
{

    struct EncodedBlock
    {
        ByteBuffer bytes;
    };

    class HuffmanEncoder
    {
    public:
        [[nodiscard]] static EncodedBlock encode(std::span<const Byte> input);
    };

}

#endif
