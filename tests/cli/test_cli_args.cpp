#include <gtest/gtest.h>

#include <vector>

#include "huffman/cli/args.hpp"

using huffman::cli::Command;
using huffman::cli::parseArgs;

namespace
{

    huffman::cli::ParsedArgs parse(std::vector<const char *> argv)
    {
        return parseArgs(static_cast<int>(argv.size()), const_cast<char **>(argv.data()));
    }

}

TEST(CliArgs, RecognizesCompress)
{
    const auto a = parse({"huffman", "compress", "in.txt", "out.huff"});
    EXPECT_EQ(a.command, Command::Compress);
    EXPECT_EQ(a.input, "in.txt");
    EXPECT_EQ(a.output, "out.huff");
}

TEST(CliArgs, RecognizesDecompress)
{
    const auto a = parse({"huffman", "decompress", "x.huff", "output"});
    EXPECT_EQ(a.command, Command::Decompress);
    EXPECT_EQ(a.input, "x.huff");
    EXPECT_EQ(a.output, "output");
}

TEST(CliArgs, NoArgsShowsHelp)
{
    const auto a = parse({"huffman"});
    EXPECT_EQ(a.command, Command::Help);
}

TEST(CliArgs, HelpFlagShowsHelp)
{
    EXPECT_EQ(parse({"huffman", "-h"}).command, Command::Help);
    EXPECT_EQ(parse({"huffman", "--help"}).command, Command::Help);
}

TEST(CliArgs, UnknownCommandIsInvalid)
{
    const auto a = parse({"huffman", "zip", "a", "b"});
    EXPECT_EQ(a.command, Command::Invalid);
    EXPECT_FALSE(a.error.empty());
}

TEST(CliArgs, TooFewArgumentsIsInvalid)
{
    const auto a = parse({"huffman", "compress", "only_input"});
    EXPECT_EQ(a.command, Command::Invalid);
    EXPECT_FALSE(a.error.empty());
}
