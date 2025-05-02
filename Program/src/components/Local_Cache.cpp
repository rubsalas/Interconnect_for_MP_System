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
