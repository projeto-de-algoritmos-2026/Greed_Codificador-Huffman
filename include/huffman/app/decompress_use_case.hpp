#ifndef HUFFMAN_APP_DECOMPRESS_USE_CASE_HPP
#define HUFFMAN_APP_DECOMPRESS_USE_CASE_HPP

#include <filesystem>

#include "huffman/ports/file_system.hpp"

namespace huffman::app
{
    class DecompressUseCase
    {
    private:
        const ports::IFileSystem &fileSystem_;

    public:
        explicit DecompressUseCase(const ports::IFileSystem &fileSystem) : fileSystem_(fileSystem) {};

        void execute(const std::filesystem::path &input, const std::filesystem::path &output) const;
    };
};

#endif
