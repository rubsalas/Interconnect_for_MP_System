#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <exception>
#include "../include/components/Local_Cache.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <cache_id> <line_index>\n";
        return 1;
    }

    try {
        uint32_t cache_id   = static_cast<uint32_t>(std::stoul(argv[1]));
        uint32_t line_index = static_cast<uint32_t>(std::stoul(argv[2]));

        // LLAMADA AL MÉTODO ESTÁTICO
        std::vector<uint8_t> bytes = LocalCache::read_cache_from_file(cache_id, line_index);

        // Imprimimos los bytes
        std::cout << "cache_" << cache_id
                  << ".txt línea " << line_index << ":\n";
        for (uint8_t b : bytes) {
            std::cout
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(b) << ' ';
        }
        std::cout << std::dec << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
    return 0;
}
