#include "../include/Instruction_Mem.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_instruction_file>\n";
        return EXIT_FAILURE;
    }

    const std::string input_filename = argv[1];
    InstructionMemory imem;

    try {
        imem.load_from_binary(input_filename);
        std::cout << "Loaded " << imem.size() << " instructions from '" << input_filename << "'.\n";
        imem.print_memory();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
