// main.cpp
#include "../include/Compiler.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;

    // Parseo simple de argumentos tipo -i <input> -o <output>
    for (int i = 1; i < argc; ++i) {
        if ((std::strcmp(argv[i], "-i") == 0 || std::strcmp(argv[i], "--ifile") == 0) && i + 1 < argc) {
            input_file = argv[++i];
        } else if ((std::strcmp(argv[i], "-o") == 0 || std::strcmp(argv[i], "--ofile") == 0) && i + 1 < argc) {
            output_file = argv[++i];
        } else if (std::strcmp(argv[i], "-h") == 0) {
            std::cout << "Uso: ./compiler -i <archivo_entrada> -o <archivo_salida>\n";
            return 0;
        }
    }

    if (input_file.empty() || output_file.empty()) {
        std::cerr << "Error: Debes proporcionar archivos de entrada y salida usando -i y -o.\n";
        return 1;
    }

    try {
        std::ifstream input(input_file);
        if (!input.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo de entrada: " + input_file);
        }

        std::vector<std::vector<std::string>> instr = clean_instructions(input);
        std::vector<std::string> bin_instr = get_binary(instr);

        std::ofstream output(output_file);
        for (const auto& line : bin_instr) {
            output << line << '\n';
        }

        std::cout << "Compilación completada. Resultado en: " << output_file << '\n';

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
