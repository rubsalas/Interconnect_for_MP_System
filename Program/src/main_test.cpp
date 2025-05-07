#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <exception>
#include "../include/components/Local_Cache.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: " << argv[0]
                  << " <cache_id> <start_line> <num_lines>\n"
                  << "Ejemplo: " << argv[0] << " 0 5 3\n";
        return 1;
    }

    try {
        uint32_t cache_id   = static_cast<uint32_t>(std::stoul(argv[1]));
        uint32_t start_line = static_cast<uint32_t>(std::stoul(argv[2]));
        uint32_t num_lines  = static_cast<uint32_t>(std::stoul(argv[3]));

        // 1) Leemos num_lines bloques
        auto bloques = LocalCache::read_cache_from_file(cache_id,
        start_line, num_lines);

        for (size_t i = 0; i < bloques.size(); ++i) {
            const auto& bytes = bloques[i];
            std::ostringstream oss;
            // Construimos la cadena hex sin espacios
            for (uint8_t b : bytes) {
                oss 
                    << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(b);
            }
            std::cout << oss.str();
            if (i + 1 < bloques.size()) {
                std::cout << ",";    // separador entre bloques
            }
        }
        std::cout << std::dec << "\n";
        // CAMBIO >>>

    }
    catch (const std::invalid_argument&) {
        std::cerr << "Error: argumentos no numéricos.\n";
        return 2;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Error de rango: " << e.what() << "\n";
        return 3;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error de E/S o formato: " << e.what() << "\n";
        return 4;
    }
    catch (const std::exception& e) {
        std::cerr << "Error inesperado: " << e.what() << "\n";
        return 5;
    }

    return 0;
}
