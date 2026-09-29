#include "huffman/app/decompress_use_case.hpp"

#include <stdexcept>

#include "huffman/core/decoder.hpp"

namespace huffman::app
{

    void DecompressUseCase::execute(const std::filesystem::path &input, const std::filesystem::path &output) const
    {
        if (input.empty())
        {
            throw std::runtime_error("Input file path is empty.");
        }

        const core::ByteBuffer compressed = fileSystem_.readFile(input);
        const core::ByteBuffer raw = core::HuffmanDecoder::decode(compressed);
        fileSystem_.writeFile(output, raw);
    }

}
