#ifndef HUFFMAN_CORE_HUFFMAN_TREE_HPP
#define HUFFMAN_CORE_HUFFMAN_TREE_HPP

#include "huffman/core/frequency_table.hpp"
#include "huffman/core/types.hpp"

namespace huffman::core
{
    class HuffmanTree
    {
    public:
        [[nodiscard]] static CodeLengths buildHuffmanCodeLengths(const FrequencyTable &frequencyTable);
        static constexpr CodeLength kMaxCodeLength = 32;
    };

}

#endif
