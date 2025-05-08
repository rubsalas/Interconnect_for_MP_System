// test_write.cpp
#include <iostream>
#include "../include/components/Local_Cache.h"
#include <iomanip>
#include <vector>
#include <cstdint>
#include <exception>


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0]
                  << " <cache_id> <start_line>\n"
                  << "Ejemplo: " << argv[0] << " 2 10\n";
        return 1;
    }

    // 1) Parsear argumentos
    uint32_t cache_id    = static_cast<uint32_t>(std::stoul(argv[1]));
    uint32_t start_line  = static_cast<uint32_t>(std::stoul(argv[2]));

    // 2) Definir las 3 líneas de cache en el propio código
    //    Cada vector interno debe tener BLOCK_SIZE = 16 bytes
    std::vector<std::vector<uint8_t>> lines = {
        // línea 0
        { 0xc8,0x0b,0x1d,0x10, 0xa6,0xdd,0x47,0xe4,
          0x7d,0xd5,0xaa,0xf1, 0x25,0xdc,0x99,0xe2 },
        // línea 1
        { 0x32,0x75,0x78,0x94, 0xf3,0xf6,0xc4,0x22,
          0x36,0x9c,0x6e,0x24, 0x59,0x2c,0x88,0x5b },
        // línea 2
        { 0xde,0xad,0xbe,0xef, 0x01,0x23,0x45,0x67,
          0x89,0xab,0xcd,0xef, 0xfe,0xdc,0xba,0x98 }
    };

    try {
        // 3) Escribir esas 3 líneas en el fichero cache_<id>.txt
        LocalCache::write_cache_lines(cache_id, start_line, lines);
        std::cout << "Se escribieron " << lines.size()
                  << " líneas en cache_" << cache_id
                  << ".txt a partir de la línea " << start_line << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error al escribir líneas de caché: "
                  << e.what() << "\n";
        return 2;
    }

    return 0;
}
