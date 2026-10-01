#include "huffman/core/encoder.hpp"

#include "huffman/core/bit_stream.hpp"
#include "huffman/core/canonical_code.hpp"
#include "huffman/core/frequency_table.hpp"
#include "huffman/core/huffman_tree.hpp"

namespace huffman::core
{
    namespace
    {
        void putUint64LE(ByteBuffer &out, std::uint64_t value)
        {
            for (int i = 0; i < 8; ++i)
            {
                out.push_back(static_cast<Byte>((value >> (i * 8)) & 0xFF));
            }
        }
    }

    EncodedBlock HuffmanEncoder::encode(std::span<const Byte> input)
    {
        FrequencyTable table;
        table.accumulate(input);

        const CodeLengths lengths = HuffmanTree::buildHuffmanCodeLengths(table);
        const CanonicalCode code = CanonicalCode::fromCodeLengths(lengths);

        EncodedBlock block;
        ByteBuffer &out = block.bytes;
        out.reserve(8 + kAlphabetSize + input.size() / 2 + 1);

        putUint64LE(out, static_cast<std::uint64_t>(input.size()));

        for (std::size_t i = 0; i < kAlphabetSize; ++i)
        {
            out.push_back(lengths[i]);
        }

        BitWriter writer(out);

        for (const Byte Bits : input)
        {
            const Code codeForBits = code.codeFor(Bits);
            writer.writeBits(codeForBits.bits, codeForBits.length);
        }

        writer.flush();

        return block;
    }
}
