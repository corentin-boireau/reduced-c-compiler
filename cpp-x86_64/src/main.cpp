#include <cstdlib>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string_view>

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

    std::string_view inputFilename(argv[1]);
    std::ifstream inputStream(inputFilename.data());
    if ( ! inputStream.is_open())
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
        if ( ! outFile.is_open())
        {
            std::cerr << "Failed to open output file \"" << outputFilename << "\"\n";
            exit(EXIT_FAILURE);
        }
        outBuf = outFile.rdbuf();
    }
    else
        outBuf = std::cout.rdbuf();

    std::ostream outputStream(outBuf);

    char c;
    while (inputStream.get(c))
        outputStream << c;
    
    return 0;
}