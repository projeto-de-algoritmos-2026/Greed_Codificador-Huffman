#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <sys/wait.h>

namespace fs = std::filesystem;

#ifndef HUFFMAN_BIN
#define HUFFMAN_BIN "huffman"
#endif

namespace {

void writeFile(const fs::path& p, const std::string& s) {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << s;
}

std::string readFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
}


// Executa o binário com os argumentos dados e devolve o código de saída do processo.
int runArgs(const std::string& args) {
    const std::string line =
        "\"" + std::string(HUFFMAN_BIN) + "\" " + args + " > /dev/null 2>&1";
    const int status = std::system(line.c_str());
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int run(const std::string& cmd, const fs::path& in, const fs::path& out) {
    return runArgs(cmd + " \"" + in.string() + "\" \"" + out.string() + "\"");
}

}

class E2ETest : public ::testing::Test {
protected:
    void SetUp() override {
        root_ = fs::temp_directory_path() /
                ("huff_e2e_" +
                 std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        fs::remove_all(root_);
        fs::create_directories(root_);
    }
    void TearDown() override {
        std::error_code ec;
        fs::remove_all(root_, ec);
    }
    fs::path root_;
};

TEST_F(E2ETest, DirectoryTreeRoundTripThroughBinary) {
    const fs::path src = root_ / "project";
    writeFile(src / "README.md", "# Project\nroot content\n");
    writeFile(src / "src/main.cpp", "int main(){ return 0; }\n");
    writeFile(src / "src/core/util.h", "#pragma once\n// util\n");
    writeFile(src / "data/table.csv", "a,b,c\n1,2,3\n4,5,6\n");
    fs::create_directories(src / "empty"); // empty directory

    const fs::path huff = root_ / "project.huff";
    const fs::path out = root_ / "restored";

    ASSERT_EQ(run("compress", src, huff), 0);
    ASSERT_TRUE(fs::exists(huff));
    ASSERT_EQ(run("decompress", huff, out), 0);

    EXPECT_EQ(readFile(out / "README.md"), readFile(src / "README.md"));
    EXPECT_EQ(readFile(out / "src/main.cpp"), readFile(src / "src/main.cpp"));
    EXPECT_EQ(readFile(out / "src/core/util.h"), readFile(src / "src/core/util.h"));
    EXPECT_EQ(readFile(out / "data/table.csv"), readFile(src / "data/table.csv"));

    EXPECT_TRUE(fs::is_directory(out / "empty"));
}

TEST_F(E2ETest, SingleFileRoundTripThroughBinary) {
    const fs::path src = root_ / "letter.txt";
    writeFile(src, "Huffman coding assigns shorter codes to more frequent symbols.");

    const fs::path huff = root_ / "letter.huff";
    const fs::path out = root_ / "letter.out";

    ASSERT_EQ(run("compress", src, huff), 0);
    ASSERT_EQ(run("decompress", huff, out), 0);
    EXPECT_EQ(readFile(out), readFile(src));
}

TEST_F(E2ETest, BinaryFileRoundTripThroughBinary) {
    std::string data;
    for (int rep = 0; rep < 50; ++rep)
        for (int b = 0; b < 256; ++b)
            data.push_back(static_cast<char>(b));
    const fs::path src = root_ / "blob.bin";
    writeFile(src, data);

    const fs::path huff = root_ / "blob.huff";
    const fs::path out = root_ / "blob.out";

    ASSERT_EQ(run("compress", src, huff), 0);
    ASSERT_EQ(run("decompress", huff, out), 0);
    EXPECT_EQ(readFile(out), data);
}

TEST_F(E2ETest, RepetitiveTextGetsSmaller) {
    std::string text;
    for (int i = 0; i < 2000; ++i)
        text += "aaaaaaabbbc";
    const fs::path src = root_ / "repetitive.txt";
    writeFile(src, text);

    const fs::path huff = root_ / "repetitive.huff";
    ASSERT_EQ(run("compress", src, huff), 0);
    EXPECT_LT(fs::file_size(huff), fs::file_size(src));
}

TEST_F(E2ETest, PathsWithSpacesWork) {
    const fs::path src = root_ / "pasta com espaco" / "arquivo com espaco.txt";
    writeFile(src, "conteudo qualquer");

    const fs::path huff = root_ / "saida com espaco.huff";
    const fs::path out = root_ / "restaurado com espaco.txt";

    ASSERT_EQ(run("compress", src, huff), 0);
    ASSERT_EQ(run("decompress", huff, out), 0);
    EXPECT_EQ(readFile(out), readFile(src));
}

TEST_F(E2ETest, HelpExitsWithZero) {
    EXPECT_EQ(runArgs(""), 0);
    EXPECT_EQ(runArgs("--help"), 0);
}

TEST_F(E2ETest, InvalidArgumentsExitWithTwo) {
    EXPECT_EQ(runArgs("zip a b"), 2);
    EXPECT_EQ(runArgs("compress only_input"), 2);
}

TEST_F(E2ETest, NonexistentInputExitsWithOne) {
    EXPECT_EQ(run("compress", root_ / "missing.txt", root_ / "x.huff"), 1);
    EXPECT_EQ(run("decompress", root_ / "missing.huff", root_ / "x.out"), 1);
}

TEST_F(E2ETest, CorruptedArchiveExitsWithOne) {
    const fs::path src = root_ / "letter.txt";
    writeFile(src, "enough text to produce a payload that can be truncated later");
    const fs::path huff = root_ / "letter.huff";
    ASSERT_EQ(run("compress", src, huff), 0);

    std::string bytes = readFile(huff);
    bytes.resize(bytes.size() - 10);
    writeFile(huff, bytes);

    EXPECT_EQ(run("decompress", huff, root_ / "letter.out"), 1);
}
