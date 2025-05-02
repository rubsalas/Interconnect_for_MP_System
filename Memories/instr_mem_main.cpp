#include "../Program/include/components/Instruction_Memory.h"
#include <iostream>
#include <bitset>

/*
 * Compile:
 * g++ -std=c++20 -Wall Program/src/components/Instruction_Memory.cpp instr_mem_main.cpp -o test_instr_mem
 * ./test_instr_mem <binary_file>
 */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo_de_instrucciones>\n";
        return 1;
    }

    std::string filename = argv[1];
    InstructionMemory mem(0);

    try {
        mem.load_from_file(filename);
    } catch (const std::exception& e) {
        std::cerr << "Error cargando archivo: " << e.what() << "\n";
        return 1;
    }

    // Mostrar las instrucciones cargadas
    for (size_t i = 0; i < mem.size(); ++i) {
        uint64_t instr = mem.fetch_instruction(i);
        std::cout << "Instruction " << i 
                  << ": 0x" << std::hex << instr 
                  << " (binary: " << std::bitset<64>(instr) << ")\n";
    }

    // Guardar las instrucciones en un archivo de volcado
    try {
        mem.dump_to_file("dump_memoria.txt");
        std::cout << "Contenido de la memoria exportado a dump_memoria.txt\n";
    } catch (const std::exception& e) {
        std::cerr << "Error al exportar memoria: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
