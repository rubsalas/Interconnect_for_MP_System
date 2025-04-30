#include "../include/Local_Cache.h"
#include <random>
#include <fstream>
#include <bitset>
#include <filesystem>

namespace fs = std::filesystem;

LocalCache::LocalCache()
    : cache_data(BLOCKS)
{}

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

void LocalCache::dump_to_text_file(const std::string& output_filename) const {
    // Asegurar que la carpeta 'caches/' existe
    fs::path dir = "caches";

    if (fs::exists(dir)) {
        if (!fs::is_directory(dir)) {
            throw std::runtime_error("Error: 'caches' existe pero no es una carpeta.");
        }
    } else {
        // No existe, la creamos
        fs::create_directories(dir);
    }

    std::ofstream out(output_filename);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo: " + output_filename);
    }

    for (const auto& block : cache_data) {
        for (const auto& byte : block) {
            out << std::bitset<8>(byte);
        }
        out << "\n";
    }

    out.close();
}
