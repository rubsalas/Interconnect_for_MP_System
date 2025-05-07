#include "../../include/components/Local_Cache.h"
#include <random>
#include <fstream>
#include <bitset>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <charconv>    // <-- para std::from_chars

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

    // Cada línea es un bloque; escribimos cada byte en hexadecimal (2 dígitos)
    for (const auto& block : cache_data) {
        for (const auto& byte : block) {
            out << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte)
                << ' ';
        }
        // Restaurar formato decimal y relleno por defecto antes de la nueva línea
        out << std::dec << std::setfill(' ') << "\n";
    }

    out.close();
}

// std::array<uint8_t, LocalCache::BLOCK_SIZE>

// LocalCache::get_cache_line(uint32_t line_index) const {
//     if (line_index >= LocalCache::BLOCKS) {
//         throw std::out_of_range(
//             "LocalCache::get_cache_line: índice de línea fuera de rango"
//         );
//     }
//     return cache_data[line_index];

// }

std::vector<uint8_t>
LocalCache::read_cache_from_file(uint32_t id, uint32_t line_index) {
    // Nombre del fichero
    std::string filename = "../config/caches/cache_" + std::to_string(id) + ".txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo " + filename);
    }

    // Avanzar hasta la línea deseada
    std::string line;
    for (uint32_t i = 0; i <= line_index; ++i) {
        if (!std::getline(file, line)) {
            throw std::out_of_range(
                "No se encontró la línea " + std::to_string(line_index));
        }
    }

    // 1) Intentamos parsear como 16 tokens hex separados por espacios
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) {
        tokens.push_back(tok);
    }

    std::vector<uint8_t> result;
    result.reserve(BLOCK_SIZE);

    if (tokens.size() == BLOCK_SIZE) {
        // caso: "ab cd ef ...", 16 tokens
        for (auto& bs : tokens) {
            uint32_t val = 0;
            auto [ptr, ec] = std::from_chars(bs.data(), bs.data() + bs.size(), val, 16);
            if (ec != std::errc() || ptr != bs.data() + bs.size()) {
                throw std::runtime_error(
                    "Token hex inválido en línea " + std::to_string(line_index) +
                    ": \"" + bs + "\"");
            }
            result.push_back(static_cast<uint8_t>(val));
        }
    }
    else {
        // caso: cadena contínua sin espacios
        // eliminamos espacios/tabulaciones etc.
        std::string raw;
        raw.reserve(line.size());
        for (char c : line) {
            if (!std::isspace(static_cast<unsigned char>(c)))
                raw.push_back(c);
        }
        if (raw.size() != BLOCK_SIZE * 2) {
            throw std::runtime_error(
                "Formato inesperado en línea " + std::to_string(line_index) +
                ": ni " + std::to_string(BLOCK_SIZE) + 
                " tokens ni " + std::to_string(BLOCK_SIZE*2) + " hex dígitos");
        }
        // parseamos pares de 2 caracteres
        for (size_t i = 0; i < raw.size(); i += 2) {
            uint32_t val = 0;
            auto begin = raw.data() + i;
            auto end   = begin + 2;
            auto [ptr, ec] = std::from_chars(begin, end, val, 16);
            if (ec != std::errc() || ptr != end) {
                throw std::runtime_error(
                    "Hex inválido en línea " + std::to_string(line_index) +
                    ", bloque en pos " + std::to_string(i/2));
            }
            result.push_back(static_cast<uint8_t>(val));
        }
    }

    // Comprobación final
    if (result.size() != BLOCK_SIZE) {
        throw std::runtime_error(
            "Se esperaban " + std::to_string(BLOCK_SIZE) +
            " bytes, pero se leyeron " + std::to_string(result.size()));
    }

    return result;
}

/* --------------------------------------------------------------------------------------------- */
