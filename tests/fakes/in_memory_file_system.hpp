#ifndef HUFFMAN_TESTS_IN_MEMORY_FILE_SYSTEM_HPP
#define HUFFMAN_TESTS_IN_MEMORY_FILE_SYSTEM_HPP

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "huffman/ports/file_system.hpp"

namespace huffman::testing
{
    class InMemoryFileSystem final : public ports::IFileSystem
    {
    private:
        static std::string key(const std::filesystem::path &p)
        {
            return p.generic_string();
        }

        void addParents(const std::string &k) const
        {
            std::filesystem::path p{k};
            p = p.parent_path();

            while (!p.empty() && p.generic_string() != "/")
            {
                dirs_.insert(p.generic_string());
                if (!p.has_parent_path())
                    break;
                p = p.parent_path();
            }
        }

        mutable std::map<std::string, core::ByteBuffer> files_;
        mutable std::set<std::string> dirs_;

    public:
        bool exists(const std::filesystem::path &p) const override
        {
            const std::string k = key(p);
            return files_.count(k) != 0 || dirs_.count(k) != 0;
        }

        bool isDirectory(const std::filesystem::path &p) const override
        {
            return dirs_.count(key(p)) != 0;
        }

        core::ByteBuffer readFile(const std::filesystem::path &p) const override
        {
            const auto it = files_.find(key(p));
            if (it == files_.end())
                throw std::runtime_error("arquivo inexistente: " + key(p));
            return it->second;
        }

        void writeFile(const std::filesystem::path &p,
                       const core::ByteBuffer &data) const override
        {
            const std::string k = key(p);
            files_[k] = data;
            addParents(k);
        }

        void createDirectories(const std::filesystem::path &p) const override
        {
            const std::string k = key(p);
            dirs_.insert(k);
            addParents(k);
        }

        std::vector<ports::FileEntry>
        listRecursive(const std::filesystem::path &root) const override
        {
            const std::string base = key(root);
            const std::string prefix = base + "/";
            std::vector<ports::FileEntry> out;

            auto collect = [&](const std::string &k, bool isDir)
            {
                if (k.rfind(prefix, 0) == 0)
                {
                    ports::FileEntry e;
                    e.relativePath = k.substr(prefix.size());
                    e.isDirectory = isDir;
                    out.push_back(std::move(e));
                }
            };
            for (const auto &[k, _] : files_)
                collect(k, false);
            for (const auto &k : dirs_)
                collect(k, true);

            std::sort(out.begin(), out.end(),
                      [](const ports::FileEntry &a, const ports::FileEntry &b)
                      {
                          return a.relativePath < b.relativePath;
                      });
            return out;
        }
    };
}

#endif
