#include "huffman/app/compress_use_case.hpp"

#include <stdexcept>

#include "huffman/app/container_format.hpp"
#include "huffman/archive/tar_packer.hpp"
#include "huffman/core/encoder.hpp"

namespace huffman::app
{

    namespace
    {
        std::vector<archive::PackedEntry> collectEntries(const ports::IFileSystem &fs, const std::filesystem::path &root)
        {
            std::vector<archive::PackedEntry> entries;
            for (const ports::FileEntry &fe : fs.listRecursive(root))
            {
                archive::PackedEntry pe;
                pe.path = fe.relativePath;
                pe.isDirectory = fe.isDirectory;
                if (!fe.isDirectory)
                {
                    pe.content = fs.readFile(root / fe.relativePath);
                }
                entries.push_back(std::move(pe));
            }
            return entries;
        }

        void writeContainer(const ports::IFileSystem &fs, const std::filesystem::path &output, Kind kind, std::span<const core::Byte> raw)
        {
            const core::EncodedBlock block = core::HuffmanEncoder::encode(raw);
            core::ByteBuffer out;
            out.reserve(block.bytes.size() + 1);
            out.push_back(static_cast<core::Byte>(kind));
            out.insert(out.end(), block.bytes.begin(), block.bytes.end());
            fs.writeFile(output, out);
        }
    }

    void CompressUseCase::execute(const std::filesystem::path &input, const std::filesystem::path &output) const
    {
        if (!fileSystem_.exists(input))
        {
            throw std::runtime_error("Input file does not exist: " + input.string());
        }
        if (fileSystem_.isDirectory(input))
        {
            const auto entries = collectEntries(fileSystem_, input);
            const core::ByteBuffer blob = archive::TarPacker::pack(entries);
            writeContainer(fileSystem_, output, Kind::Directory, blob);
        }
        else
        {
            const core::ByteBuffer raw = fileSystem_.readFile(input);
            writeContainer(fileSystem_, output, Kind::File, raw);
        }
    }
}
