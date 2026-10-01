#include <iostream>

#include "huffman/app/compress_use_case.hpp"
#include "huffman/app/decompress_use_case.hpp"
#include "huffman/cli/args.hpp"
#include "huffman/infra/std_file_system.hpp"

namespace
{

    void printUsage()
    {
        std::cout
            << "huffman - compressor de dados (Código de Huffman)\n\n"
            << "Uso:\n"
            << "  huffman compress   <entrada> <saída.huff>\n"
            << "  huffman decompress <entrada.huff> <saída>\n\n"
            << "A entrada de 'compress' pode ser um arquivo ou uma pasta\n"
            << "(com subpastas). Funciona com qualquer tipo de arquivo.\n";
    }

} // namespace

int main(int argc, char **argv)
{
    const huffman::cli::ParsedArgs args = huffman::cli::parseArgs(argc, argv);

    switch (args.command)
    {
    case huffman::cli::Command::Help:
        printUsage();
        return 0;

    case huffman::cli::Command::Invalid:
        std::cerr << "Erro: " << args.error << "\n\n";
        printUsage();
        return 2;

    case huffman::cli::Command::Compress:
    case huffman::cli::Command::Decompress:
        break; // segue abaixo
    }

    huffman::infra::StdFileSystem fs;

    try
    {
        if (args.command == huffman::cli::Command::Compress)
        {
            huffman::app::CompressUseCase{fs}.execute(args.input, args.output);
            std::cout << "Comprimido: " << args.output << "\n";
        }
        else
        {
            huffman::app::DecompressUseCase{fs}.execute(args.input, args.output);
            std::cout << "Descomprimido: " << args.output << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
