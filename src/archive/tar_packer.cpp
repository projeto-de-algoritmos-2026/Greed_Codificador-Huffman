#include "huffman/archive/tar_packer.hpp"

#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>

namespace huffman::archive
{
    namespace
    {
        using core::Byte;
        using core::ByteBuffer;

        constexpr char kMagic[4] = {'H', 'T', 'A', 'R'};
        constexpr std::size_t kMinEntrySize = 1 + 2 + 8;

        void putU16(ByteBuffer &out, std::uint16_t value)
        {
            out.push_back(static_cast<Byte>(value & 0xFF));
            out.push_back(static_cast<Byte>((value >> 8) & 0xFF));
        }

        void putU32(ByteBuffer &out, std::uint32_t value)
        {
            for (int i = 0; i < 4; ++i)
            {
                out.push_back(static_cast<Byte>((value >> (i * 8)) & 0xFF));
            }
        }

        void putU64(ByteBuffer &out, std::uint64_t value)
        {
            for (int i = 0; i < 8; ++i)
            {
                out.push_back(static_cast<Byte>((value >> (i * 8)) & 0xFF));
            }
        }

        class Reader
        {
        private:
            std::span<const Byte> data_;
            std::size_t pos_ = 0;

        public:
            explicit Reader(std::span<const Byte> data) : data_(data) {}

            [[nodiscard]] bool atEnd() const
            {
                return pos_ == data_.size();
            }

            void need(std::size_t size)
            {
                if (size > data_.size() - pos_)
                {
                    throw std::runtime_error("Unexpected end of data");
                }
            }

            std::uint16_t readU16()
            {
                need(2);
                std::uint16_t value = static_cast<std::uint16_t>(data_[pos_] | (data_[pos_ + 1] << 8));
                pos_ += 2;
                return value;
            }

            std::uint32_t readU32()
            {
                need(4);
                std::uint32_t value = 0;
                for (int i = 0; i < 4; ++i)
                    value |= static_cast<std::uint32_t>(data_[pos_ + static_cast<std::size_t>(i)]) << (i * 8);
                pos_ += 4;
                return value;
            }

            std::uint64_t readU64()
            {
                need(8);
                std::uint64_t value = 0;
                for (int i = 0; i < 8; ++i)
                    value |= static_cast<std::uint64_t>(data_[pos_ + static_cast<std::size_t>(i)]) << (i * 8);
                pos_ += 8;
                return value;
            }

            Byte byte()
            {
                need(1);
                return data_[pos_++];
            }

            ByteBuffer bytes(std::size_t n)
            {
                need(n);
                ByteBuffer byte(data_.begin() + static_cast<std::ptrdiff_t>(pos_),
                                data_.begin() + static_cast<std::ptrdiff_t>(pos_ + n));
                pos_ += n;
                return byte;
            }
        };

    }

    ByteBuffer TarPacker::pack(const std::vector<PackedEntry> &entries)
    {
        ByteBuffer out;
        out.insert(out.end(), kMagic, kMagic + 4);
        putU32(out, static_cast<std::uint32_t>(entries.size()));

        for (const PackedEntry &entity : entries)
        {
            if (entity.path.size() > std::numeric_limits<std::uint16_t>::max())
                throw std::length_error("Path too long: " + entity.path.substr(0, 32) + "...");
            if (entity.isDirectory && !entity.content.empty())
                throw std::invalid_argument("Directory entry cannot have content: " + entity.path);

            out.push_back(entity.isDirectory ? Byte{1} : Byte{0});
            putU16(out, static_cast<std::uint16_t>(entity.path.size()));
            out.insert(out.end(), entity.path.begin(), entity.path.end());
            putU64(out, static_cast<std::uint64_t>(entity.content.size()));
            out.insert(out.end(), entity.content.begin(), entity.content.end());
        }
        return out;
    }

    std::vector<PackedEntry> TarPacker::unpack(std::span<const Byte> blob)
    {
        Reader reader(blob);

        const ByteBuffer magic = reader.bytes(4);
        if (std::memcmp(magic.data(), kMagic, 4) != 0)
            throw std::runtime_error("Invalid magic number");

        const std::uint32_t count = reader.readU32();
        if (count > blob.size() / kMinEntrySize)
            throw std::runtime_error("Invalid entry count");

        std::vector<PackedEntry> entries;
        entries.reserve(count);

        for (std::uint32_t i = 0; i < count; ++i)
        {
            PackedEntry entity;
            entity.isDirectory = (reader.byte() != 0);

            const std::uint16_t pathLen = reader.readU16();
            const ByteBuffer pathBytes = reader.bytes(pathLen);
            const std::uint64_t contentLen = reader.readU64();
            if (entity.isDirectory && contentLen != 0)
                throw std::runtime_error("Directory entry cannot have content");

            entity.path.assign(pathBytes.begin(), pathBytes.end());
            entity.content = reader.bytes(static_cast<std::size_t>(contentLen));

            entries.push_back(std::move(entity));
        }

        if (!reader.atEnd())
            throw std::runtime_error("Unexpected trailing data");
        return entries;
    }
}
