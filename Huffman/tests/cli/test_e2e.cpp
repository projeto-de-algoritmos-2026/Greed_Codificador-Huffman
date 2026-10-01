#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

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


int run(const std::string& cmd, const fs::path& in, const fs::path& out) {
    const std::string line =
        std::string(HUFFMAN_BIN) + " " + cmd + " \"" + in.string() +
        "\" \"" + out.string() + "\"";
    return std::system(line.c_str());
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