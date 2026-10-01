#include "huffman/core/canonical_code.hpp"

namespace huffman::core
{

    CanonicalCode CanonicalCode::fromCodeLengths(const CodeLengths &lengths)
    {
        CanonicalCode canonical_code;
        canonical_code.lengths_ = lengths;

        CodeLength maxLen = 0;
        for (std::size_t s = 0; s < kAlphabetSize; ++s)
        {
            const CodeLength len = lengths[s];
            if (len > 0)
            {
                canonical_code.countByLength_[len]++;
                if (len > maxLen)
                    maxLen = len;
            }
        }
        canonical_code.maxLength_ = maxLen;
        if (maxLen == 0)
            return canonical_code;

        std::uint32_t code = 0;
        for (CodeLength len = 1; len <= maxLen; ++len)
        {
            code = (code + canonical_code.countByLength_[len - 1]) << 1;
            canonical_code.firstCode_[len] = code;
        }

        std::uint32_t running = 0;
        for (CodeLength len = 1; len <= maxLen; ++len)
        {
            canonical_code.firstIndex_[len] = running;
            running += canonical_code.countByLength_[len];
        }

        std::array<std::uint32_t, kMaxUsableLength + 1> nextCode = canonical_code.firstCode_;
        std::array<std::uint32_t, kMaxUsableLength + 1> fillPos = canonical_code.firstIndex_;
        for (std::size_t s = 0; s < kAlphabetSize; ++s)
        {
            const CodeLength len = lengths[s];
            if (len == 0)
                continue;
            canonical_code.codes_[s] = Code{nextCode[len]++, len};
            canonical_code.sortedSymbols_[fillPos[len]++] = static_cast<Byte>(s);
        }

        return canonical_code;
    }

}
