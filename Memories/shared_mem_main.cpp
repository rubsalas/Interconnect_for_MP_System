#include "../Program/include/components/Shared_Memory.h"
#include <iostream>

/*
 * Compile:
 * g++ -std=c++20 -Wall Program/src/components/Shared_Memory.cpp shared_mem_main.cpp -o test_shared_mem
 * ./test_shared_mem <cant. PEs>
 */

int main() {
    SharedMemory mem;

    // Llenar memoria con valores aleatorios
    mem.fill_random();

    // Guardar en binario plano
    try {
        mem.dump_to_binary_file("dump_shared_memory.bin");
        std::cout << "Memoria de datos exportada a dump_shared_memory.bin\n";
    } catch (const std::exception& e) {
        std::cerr << "Error al exportar memoria binaria: " << e.what() << "\n";
    }

    // Guardar en texto binario (string)
    try {
        mem.dump_to_text_file("dump_shared_memory.txt");
        std::cout << "Memoria de datos exportada a dump_shared_memory.txt\n";
    } catch (const std::exception& e) {
        std::cerr << "Error al exportar memoria de texto: " << e.what() << "\n";
    }

    return 0;
}
