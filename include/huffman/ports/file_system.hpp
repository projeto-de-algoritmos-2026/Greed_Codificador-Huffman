#ifndef HUFFMAN_PORTS_FILE_SYSTEM_HPP
#define HUFFMAN_PORTS_FILE_SYSTEM_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "huffman/core/types.hpp"

namespace huffman::ports
{

    struct FileEntry
    {
        std::string relativePath;
        bool isDirectory = false;
    };

    class IFileSystem
    {
    public:
        virtual ~IFileSystem() = default;

        [[nodiscard]] virtual bool exists(const std::filesystem::path &p) const = 0;
        [[nodiscard]] virtual bool isDirectory(const std::filesystem::path &p) const = 0;

        [[nodiscard]] virtual std::vector<FileEntry>
        listRecursive(const std::filesystem::path &root) const = 0;

        [[nodiscard]] virtual core::ByteBuffer
        readFile(const std::filesystem::path &p) const = 0;

        virtual void writeFile(const std::filesystem::path &p,
                               const core::ByteBuffer &data) const = 0;

        virtual void createDirectories(const std::filesystem::path &p) const = 0;
    };
}

#endif
