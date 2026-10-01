#include "huffman/cli/args.hpp"

#include <string_view>
#include <vector>

namespace huffman::cli
{
    ParsedArgs parseArgs(int argc, char **argv)
    {
        ParsedArgs result;

        std::vector<std::string_view> args;
        for (int i = 1; i < argc; ++i)
            args.emplace_back(argv[i]);

        if (args.empty() || args[0] == "-h" || args[0] == "--help")
        {
            result.command = Command::Help;
            return result;
        }

        const std::string_view cmd = args[0];
        if (cmd != "compress" && cmd != "decompress")
        {
            result.command = Command::Invalid;
            result.error = "Unknown command: " + std::string(cmd);
            return result;
        }

        if (args.size() != 3)
        {
            result.command = Command::Invalid;
            result.error = "Usage: " + std::string(cmd) + " <input> <output>";
            return result;
        }

        result.command =
            (cmd == "compress") ? Command::Compress : Command::Decompress;
        result.input = std::string(args[1]);
        result.output = std::string(args[2]);
        return result;
    }
}
