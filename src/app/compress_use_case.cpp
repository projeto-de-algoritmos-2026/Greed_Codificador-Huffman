#include "huffman/app/compress_use_case.hpp"

#include <stdexcept>

#include "huffman/core/encoder.hpp"

namespace huffman::app
{

    void CompressUseCase::execute(const std::filesystem::path &input, const std::filesystem::path &output) const
    {
        if (!fileSystem_.exists(input))
        {
            throw std::runtime_error("Input file does not exist: " + input.string());
        }

        const core::ByteBuffer raw = fileSystem_.readFile(input);
        const core::EncodedBlock block = core::HuffmanEncoder::encode(raw);
        fileSystem_.writeFile(output, block.bytes);
    }

}
