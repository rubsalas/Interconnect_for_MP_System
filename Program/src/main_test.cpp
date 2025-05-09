// #include "../include/components/Shared_Memory.h"
// #include <iostream>
// #include <vector>
// #include <cstdlib>
// #include <cmath>

// int main(int argc, char* argv[]) {
//     if (argc != 3) {
//         std::cerr << "Uso: " << argv[0] << " <address> <size_bytes>\n";
//         std::cerr << "  - address: línea de memoria desde donde leer (entero >= 0)\n";
//         std::cerr << "  - size_bytes: cantidad de bytes a leer desde esa dirección\n";
//         return 1;
//     }

//     // Leer argumentos
//     size_t address = std::strtoul(argv[1], nullptr, 10);
//     size_t size_bytes = std::strtoul(argv[2], nullptr, 10);

//     // Crear instancia de memoria
//     SharedMemory shared_mem;

//     // Leer bloques desde el archivo shared_memory.txt
//     auto blocks = shared_mem.read_shared_memory(address, size_bytes);

//     // Calcular cuántas líneas se deben mostrar (size_bytes / 4)
//     size_t lines_to_read = static_cast<size_t>(std::ceil(size_bytes / 4.0));
//     size_t total_lines_printed = 0;

//     std::cout << "\nLectura desde shared_memory.txt (" << size_bytes << " bytes desde línea " << address << "):\n";

//     for (size_t i = 0; i < blocks.size(); ++i) {
//         for (size_t j = 0; j < 4 && total_lines_printed < lines_to_read; ++j) {
//             size_t mem_line = address + total_lines_printed;
//             std::cout << "Línea " << mem_line << ": " << blocks[i][j] << "\n";
//             ++total_lines_printed;
//         }
//     }

//     return 0;
// }
