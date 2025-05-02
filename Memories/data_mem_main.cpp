#include "Program/include/components/Data_Memory.h"
#include <iostream>

/*
 * Compile:
 * g++ -std=c++20 -Wall Program/src/components/Data_Memory.cpp data_mem_main.cpp -o test_data_mem
 * ./test_data_mem <cant. PEs>
 */

int main() {
    DataMemory mem;

    // Llenar memoria con valores aleatorios
    mem.fill_random();

    // Guardar en binario plano
    try {
        mem.dump_to_binary_file("dump_data_memory.bin");
        std::cout << "Memoria de datos exportada a dump_data_memory.bin\n";
    } catch (const std::exception& e) {
        std::cerr << "Error al exportar memoria binaria: " << e.what() << "\n";
    }

    // Guardar en texto binario (string)
    try {
        mem.dump_to_text_file("dump_data_memory.txt");
        std::cout << "Memoria de datos exportada a dump_data_memory.txt\n";
    } catch (const std::exception& e) {
        std::cerr << "Error al exportar memoria de texto: " << e.what() << "\n";
    }

    return 0;
}
