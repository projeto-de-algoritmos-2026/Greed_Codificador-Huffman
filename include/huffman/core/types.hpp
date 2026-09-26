#ifndef HUFFMAN_CORE_TYPES_HPP
#define HUFFMAN_CORE_TYPES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace huffman::core
{
    using Byte = std::uint8_t;
    using ByteBuffer = std::vector<Byte>;

    inline constexpr std::size_t kAlphabetSize = 256;
}

#endif
