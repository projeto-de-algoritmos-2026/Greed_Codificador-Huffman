#ifndef HUFFMAN_ARCHIVE_TAR_PACKER_HPP
#define HUFFMAN_ARCHIVE_TAR_PACKER_HPP

#include <span>
#include <string>
#include <vector>

#include "huffman/core/types.hpp"

namespace huffman::archive
{
    struct PackedEntry
    {
        std::string path;
        bool isDirectory = false;
        core::ByteBuffer content;
    };

    class TarPacker
    {
    public:
        [[nodiscard]] static core::ByteBuffer pack(const std::vector<PackedEntry> &entries);
        [[nodiscard]] static std::vector<PackedEntry> unpack(std::span<const core::Byte> blob);
    };
}

#endif
