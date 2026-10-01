#ifndef HUFFMAN_CLI_ARGS_HPP
#define HUFFMAN_CLI_ARGS_HPP

#include <string>

namespace huffman::cli
{
    enum class Command
    {
        Compress,
        Decompress,
        Help,
        Invalid,
    };

    struct ParsedArgs
    {
        Command command = Command::Invalid;
        std::string input;
        std::string output;
        std::string error;
    };

    [[nodiscard]] ParsedArgs parseArgs(int argc, char **argv);
}

#endif
