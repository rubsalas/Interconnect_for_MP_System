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
#include <charconv>

namespace fs = std::filesystem;

LocalCache::LocalCache(int id)
    : id_(id), cache_data(BLOCKS) {
    std::cout << "\n[LocalCache] PE " << id_
              << ": creating cache with " << BLOCKS
              << " blocks of " << BLOCK_SIZE << " bytes each...\n";

    // Guarda el directorio y el filename donde se volcara el cache en disco
    dump_path = "config/caches/cache_" + std::to_string(id_) + ".txt";

    inv_path = "config/caches/inv_cache_" + std::to_string(id_) + ".txt";

    // Inicializar Cache
    initialize();
}

/* ---------------------------------------- Initializing --------------------------------------- */

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

    // Crear archivo cache_inv_<pe>.txt
    std::ofstream inv_file(inv_path);
    if (!inv_file.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo: " + inv_path);
    }

    // Cada línea es un bloque; escribimos cada byte en hexadecimal (2 dígitos)
    for (const auto& block : cache_data) {
        for (const auto& byte : block) {
            out << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte);
               // << ' ';
        }
        // Restaurar formato decimal y relleno por defecto antes de la nueva línea
        out << std::dec << std::setfill(' ') << "\n";
        inv_file << "0\n";
    }

    out.close();
    inv_file.close();
}

/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------- Data Handling --------------------------------------- */ 

std::vector<std::vector<uint8_t>> LocalCache::read_cache_from_file(uint32_t id,
                                                                    uint32_t start_line,
                                                                    uint32_t num_lines) {
    // 1) Abrir el fichero
    std::string filename = "config/caches/cache_" + std::to_string(id) + ".txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo " + filename);
    }

    std::string line;
    // 2) Saltar hasta la línea inicial
    for (uint32_t i = 0; i < start_line; ++i) {
        if (!std::getline(file, line)) {
            throw std::out_of_range(
                "No se encontró la línea " + std::to_string(i)
            );
        }
    }

    // 3) Leer y parsear cada línea
    std::vector<std::vector<uint8_t>> result;
    result.reserve(num_lines);

    for (uint32_t ln = 0; ln < num_lines; ++ln) {
        if (!std::getline(file, line)) {
            throw std::out_of_range(
                "No se encontró la línea " +
                std::to_string(start_line + ln)
            );
        }

        // dividimos por espacios
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) tokens.push_back(token);

        std::vector<uint8_t> bytes;
        bytes.reserve(LocalCache::BLOCK_SIZE);

        if (tokens.size() == LocalCache::BLOCK_SIZE) {
            // caso: 16 tokens “ab cd ef …”
            for (auto& bs : tokens) {
                uint32_t val = 0;
                auto [ptr, ec] = std::from_chars(bs.data(),
                                                bs.data() + bs.size(),
                                                val,
                                                16);
                if (ec != std::errc() || ptr != bs.data() + bs.size()) {
                    throw std::runtime_error(
                        "Token hex inválido en línea " +
                        std::to_string(start_line + ln) +
                        ": \"" + bs + "\""
                    );
                }
                bytes.push_back(static_cast<uint8_t>(val));
            }
        } else {
            // caso: 32 dígitos contiguos “c80b…”
            std::string raw;
            raw.reserve(line.size());
            for (char c : line) {
                if (!std::isspace(static_cast<unsigned char>(c)))
                    raw.push_back(c);
            }
            if (raw.size() != LocalCache::BLOCK_SIZE * 2) {
                throw std::runtime_error(
                    "Formato inesperado en línea " +
                    std::to_string(start_line + ln)
                );
            }
            for (size_t i = 0; i < raw.size(); i += 2) {
                uint32_t val = 0;
                auto [ptr, ec] = std::from_chars(raw.data() + i,
                                                raw.data() + i + 2,
                                                val,
                                                16);
                if (ec != std::errc() || ptr != raw.data() + i + 2) {
                    throw std::runtime_error(
                        "Hex inválido en línea " +
                        std::to_string(start_line + ln)
                    );
                }
                bytes.push_back(static_cast<uint8_t>(val));
            }
        }

        result.push_back(std::move(bytes));
    }

    return result;
}

void LocalCache::write_cache_lines(uint32_t id, uint32_t start_line,
                                    const std::vector<std::vector<uint8_t>>& lines) {
    // 1) Ruta al fichero
    std::string filename = "config/caches/cache_" + std::to_string(id) + ".txt";

    // 2) Leer todo el fichero en memoria
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo " + filename);
    }
    std::vector<std::string> file_lines;
    std::string line;
    while (std::getline(infile, line)) {
        file_lines.push_back(line);
    }
    infile.close();

    // 3) Validar rango
    if (start_line >= file_lines.size() ||
        start_line + lines.size() > file_lines.size()) {
        throw std::out_of_range(
        "LocalCache::write_cache_lines: líneas fuera de rango"
        );
    }

    // 4) Reemplazar cada línea por la nueva
    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& bytes = lines[i];
        if (bytes.size() != BLOCK_SIZE) {
            throw std::invalid_argument(
            "LocalCache::write_cache_lines: cada línea debe tener " +
            std::to_string(BLOCK_SIZE) + " bytes"
            );
        }
        std::ostringstream oss;
        for (uint8_t b : bytes) {
            oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(b);
        }
        file_lines[start_line + i] = oss.str();
    }

    // 5) Volcar de nuevo al disco
    std::ofstream outfile(filename, std::ios::trunc);
    if (!outfile.is_open()) {
        throw std::runtime_error("No se pudo escribir en el archivo " + filename);
    }
    for (const auto& l : file_lines) {
        outfile << l << "\n";
    }
    outfile.close();
}

void LocalCache::invalidate_line(uint32_t line_index, int pe_id) {
    // Leer todas las líneas del archivo de invalidación
    std::string inv_path = "config/caches/inv_cache_" + std::to_string(pe_id) + ".txt";
    std::ifstream infile(inv_path);
    if (!infile.is_open()) {
        std::cerr << "[LocalCache] No se pudo abrir " << inv_path << " para lectura.\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // Validar índice
    if (line_index < 0 || static_cast<size_t>(line_index) >= lines.size()) {
        std::cerr << "[LocalCache] Índice de línea inválido: " << line_index << "\n";
        return;
    }

    // Cambiar la línea a "1"
    lines[line_index] = "1";

    // Reescribir el archivo completo con la línea actualizada
    std::ofstream outfile(inv_path);
    if (!outfile.is_open()) {
        std::cerr << "[LocalCache] No se pudo abrir " << inv_path << " para escritura.\n";
        return;
    }

    for (const auto& l : lines) {
        outfile << l << "\n";
    }

    std::cout << "[LocalCache] Línea " << line_index << " invalidada exitosamente.\n";
}

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

void LocalCache::read_test(uint32_t start_line, uint32_t num_lines) const {
    // Verificar que el rango [start_line, start_line + num_lines) sea válido
    std::cout << "[LocalCache] PE " << id_
              << ": reading lines " << start_line
              << " to " << (start_line + num_lines - 1)
              << "\n";
    if (start_line >= BLOCKS || start_line + num_lines > BLOCKS) {
        throw std::out_of_range(
            "LocalCache::read: rango fuera de límites");
    }

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
