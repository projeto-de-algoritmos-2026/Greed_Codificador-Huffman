#ifndef HUFFMAN_INFRA_STD_FILE_SYSTEM_HPP
#define HUFFMAN_INFRA_STD_FILE_SYSTEM_HPP

#include "huffman/ports/file_system.hpp"

namespace huffman::infra
{
    class StdFileSystem final : public huffman::ports::IFileSystem
    {
    public:
        [[nodiscard]] bool exists(const std::filesystem::path &p) const override;
        [[nodiscard]] bool isDirectory(const std::filesystem::path &p) const override;
        [[nodiscard]] std::vector<huffman::ports::FileEntry> listRecursive(const std::filesystem::path &root) const override;
        [[nodiscard]] huffman::core::ByteBuffer readFile(const std::filesystem::path &p) const override;
        void writeFile(const std::filesystem::path &p, const huffman::core::ByteBuffer &data) const override;
        void createDirectories(const std::filesystem::path &p) const override;
    };
};

#endif
