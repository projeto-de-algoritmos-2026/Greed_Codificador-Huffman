#include "huffman/infra/std_file_system.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace huffman::infra
{
    namespace fs = std::filesystem;

    bool StdFileSystem::exists(const fs::path &p) const
    {
        return fs::exists(p);
    };

    bool StdFileSystem::isDirectory(const fs::path &p) const
    {
        return fs::is_directory(p);
    };

    std::vector<ports::FileEntry> StdFileSystem::listRecursive(const fs::path &root) const
    {
        std::vector<ports::FileEntry> entries;
        for (const auto &e : fs::recursive_directory_iterator(root))
        {
            ports::FileEntry fe;
            fe.relativePath = fs::relative(e.path(), root).generic_string();
            fe.isDirectory = e.is_directory();
            entries.push_back(std::move(fe));
        }

        std::sort(entries.begin(), entries.end(), [](const ports::FileEntry &a, const ports::FileEntry &b)
                  { return a.relativePath < b.relativePath; });

        return entries;
    }

    core::ByteBuffer StdFileSystem::readFile(const fs::path &p) const
    {
        std::ifstream in(p, std::ios::binary);

        if (!in)
        {
            throw std::runtime_error("Could not open file for reading: " + p.string());
        }

        return core::ByteBuffer(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

    void StdFileSystem::writeFile(const fs::path &p, const core::ByteBuffer &data) const
    {
        if (p.has_parent_path())
        {
            fs::create_directories(p.parent_path());
        }

        std::ofstream out(p, std::ios::binary);

        if (!out)
        {
            throw std::runtime_error("Could not open file for writing: " + p.string());
        }

        out.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    }

    void StdFileSystem::createDirectories(const fs::path &p) const
    {
        fs::create_directories(p);
    }
}
