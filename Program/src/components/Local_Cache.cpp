#include "../../include/components/Local_Cache.h"
#include <random>
#include <fstream>
#include <bitset>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

LocalCache::LocalCache(int id)
    : id_(id), cache_data(BLOCKS) {
        std::cout << "\n[LocalCache] PE " << id_ 
              << ": creating cache with " << BLOCKS 
              << " blocks of " << BLOCK_SIZE << " bytes each...\n";

        // Guarda el directorio y el filename donde se volcara el cache en disco
        dump_path = "config/caches/cache_" + std::to_string(id_) + ".txt";

        // Inicializar Cache
        initialize();
}

void LocalCache::initialize() {
    // Se llena con datos aleatorios
    fill_random();

    // Volcado del cache a disco
    dump_to_text_file();
}

void LocalCache::fill_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> distrib(0, 255);

    for (auto& block : cache_data) {
        for (auto& byte : block) {
            byte = distrib(gen);
        }
    }
}

void LocalCache::dump_to_text_file() const {
    // Asegurar que la carpeta existe
    fs::path dir = "config/caches";

    if (fs::exists(dir)) {
        if (!fs::is_directory(dir)) {
            throw std::runtime_error("Error: 'caches' existe pero no es una carpeta.");
        }
    } else {
        // No existe, se crea
        fs::create_directories(dir);
    }
    
    std::ofstream out(dump_path);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo: " + dump_path);
    }

    for (const auto& block : cache_data) {
        for (const auto& byte : block) {
            out << std::bitset<8>(byte);
        }
        out << "\n";
    }

    out.close();
}

/* ---------------------------------------- Testing -------------------------------------------- */

void LocalCache::read_test(uint32_t start_line, uint32_t num_lines) const {
    // Verificar que el rango [start_line, start_line + num_lines) sea válido
    if (start_line >= BLOCKS || start_line + num_lines > BLOCKS) {
        throw std::out_of_range(
            "LocalCache::read: rango fuera de límites");
    }

    std::cout << "[LocalCache] PE " << id_
              << ": reading lines " << start_line
              << " to " << (start_line + num_lines - 1)
              << "\n";

    // Para cada línea solicitada…
    /*for (uint32_t offset = 0; offset < num_lines; ++offset) {
        uint32_t line = start_line + offset;

        // Imprimir índice de línea
        std::cout << "  Line " << std::setw(3) << line << ":";

        // Cada byte de la línea en hexadecimal con dos dígitos
        for (uint8_t byte : cache_data[line]) {
            std::cout << " 0x"
                      << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte);
        }

        // Restaurar formato y terminar línea
        std::cout << std::dec
                  << std::setfill(' ')
                  << "\n";
    }*/

    std::cout << std::endl;
}

/* --------------------------------------------------------------------------------------------- */
