#include "huffman/app/decompress_use_case.hpp"

#include <stdexcept>

#include "huffman/app/container_format.hpp"
#include "huffman/archive/tar_packer.hpp"
#include "huffman/core/decoder.hpp"

namespace huffman::app
{
    void DecompressUseCase::execute(const std::filesystem::path &input, const std::filesystem::path &output) const
    {
        if (!fileSystem_.exists(input))
        {
            throw std::runtime_error("Input file does not exist: " + input.string());
        }

        const core::ByteBuffer container = fileSystem_.readFile(input);
        if (container.empty())
        {
            throw std::runtime_error("Compressed file is empty: " + input.string());
        }

        const Kind kind = static_cast<Kind>(container.front());
        const std::span<const core::Byte> block{container.data() + 1, container.size() - 1};
        const core::ByteBuffer raw = core::HuffmanDecoder::decode(block);

        if (kind == Kind::File)
        {
            fileSystem_.writeFile(output, raw);
            return;
        }

        const auto entries = archive::TarPacker::unpack(raw);
        fileSystem_.createDirectories(output);
        for (const archive::PackedEntry &e : entries)
        {
            const std::filesystem::path dst = output / e.path;
            if (e.isDirectory)
            {
                fileSystem_.createDirectories(dst);
            }
            else
            {
                fileSystem_.writeFile(dst, e.content);
            }
        }
    }
}
