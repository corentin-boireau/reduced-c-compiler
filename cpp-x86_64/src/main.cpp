#include <cstdlib>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string_view>

#include "Tokenizer.h"

void print_usage(std::ostream& outStream)
{
    outStream <<
        "RCC usage:\n"
        "rcc <input_file> [<output_file>]\n"
        ;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Not enough arguments." << std::endl;
        print_usage(std::cerr);
        exit(EXIT_FAILURE);
    }

    std::string inputFilename(argv[1]);
    std::ifstream inputStream(inputFilename);
    if (not inputStream.is_open())
    {
        std::cerr << "Failed to open input file \"" << inputFilename << "\"\n";
        exit(EXIT_FAILURE);
    }

    std::streambuf* outBuf;
    std::ofstream outFile;
    if (argc >= 3)
    {
        std::string_view outputFilename(argv[2]);
        outFile.open(outputFilename.data());
        if (not outFile.is_open())
        {
            std::cerr << "Failed to open output file \"" << outputFilename << "\"\n";
            exit(EXIT_FAILURE);
        }
        outBuf = outFile.rdbuf();
    }
    else
        outBuf = std::cout.rdbuf();

    std::ostream outputStream(outBuf);
    
    Tokenizer tokenizer(std::move(inputFilename), inputStream);

    for (token_t const& token : tokenizer)
        outputStream << "lol\n";

    char c;
    while (inputStream.get(c))
        outputStream << c;
    
    return 0;
}