#include "../../include/components/Local_Cache.h"
#include <random>
#include <fstream>
#include <bitset>
#include <filesystem>
#include <iostream>
#include <iomanip> 

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

LocalCache::LocalCache(int id, const std::string& file_path)
  : id_(id)
  , cache_data(BLOCKS)
  , dump_path(file_path)
{
    // no llamamos a dump_to_text_file(), sólo preparamos la ruta
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
    // 1) Asegurarse de que exista el directorio donde volcar
    fs::path p(dump_path);
    fs::create_directories(p.parent_path());

    // 2) Abrir el fichero en modo escritura (trunca si ya existe)
    std::ofstream out(dump_path, std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo: " + dump_path);
    }

    // 3) Volcar cada bloque como una línea de 16 bytes en hex (32 dígitos)
    for (const auto& block : cache_data) {
        for (const auto& byte : block) {
            out << std::hex
                << std::setw(2)
                << std::setfill('0')
                << static_cast<unsigned>(byte);
        }
        out << std::dec  // restaurar base decimal para seguridad
            << "\n";
    }

    out.close();
}


