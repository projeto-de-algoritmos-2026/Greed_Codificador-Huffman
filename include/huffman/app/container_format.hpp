#ifndef HUFFMAN_APP_CONTAINER_FORMAT_HPP
#define HUFFMAN_APP_CONTAINER_FORMAT_HPP

#include "huffman/core/types.hpp"

namespace huffman::app
{
    enum class Kind : core::Byte
    {
        File = 0,
        Directory = 1,
    };
}

#endif
