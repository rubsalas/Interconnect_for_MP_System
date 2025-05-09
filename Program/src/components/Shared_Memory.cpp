#include "../../include/components/Shared_Memory.h"
#include <random>
#include <fstream>
#include <bitset>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

SharedMemory::SharedMemory()
    : data(MEMORY_SIZE, 0) {
        std::cout << "[SharedMemory] Initializing shared memory with "
              << MEMORY_SIZE << " words of 32 bits each...\n";

        // Guarda el directorio y el filename donde se volcara el shared memory en disco
        dump_path_txt = "../config/shared_memory/shared_memory.txt";
        dump_path_bin = "../config/shared_memory/shared_memory.bin";

        // Inicializar Shared Memory
        initialize();
}

void SharedMemory::initialize() {
    // Se llena con datos aleatorios
    fill_random();

    // Volcado del shared memory como binario a disco
    dump_to_binary_file();

    // Volcado del shared memory como text a disco
    dump_to_text_file();
}

uint32_t SharedMemory::load(size_t address) const {
    if (address >= MEMORY_SIZE) {
        throw std::out_of_range("Error: Dirección de memoria fuera de rango en load().");
    }
    return data[address];
}

void SharedMemory::store(size_t address, uint32_t value) {
    if (address >= MEMORY_SIZE) {
        throw std::out_of_range("Error: Dirección de memoria fuera de rango en store().");
    }
    data[address] = value;
}

size_t SharedMemory::size() const {
    return MEMORY_SIZE;
}

void SharedMemory::fill_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> distrib(0, UINT32_MAX);

    for (auto& elem : data) {
        elem = distrib(gen);
    }
}

void SharedMemory::dump_to_binary_file() const {
    // Asegurar que la carpeta existe
    fs::path dir = "config/shared_memory";
    if (fs::exists(dir)) {
        if (!fs::is_directory(dir)) {
            throw std::runtime_error("Error: 'shared_memory' existe pero no es una carpeta.");
        }
    } else {
        // No existe, se crea
        fs::create_directories(dir);
    }

    std::ofstream out(dump_path_bin, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo binario: " + dump_path_bin);
    }
    out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint32_t));
    out.close();
}

void SharedMemory::dump_to_text_file() const {
    // Asegurar que la carpeta existe
    fs::path dir = "../config/shared_memory";
    if (fs::exists(dir)) {
        if (!fs::is_directory(dir)) {
            throw std::runtime_error("Error: 'shared_memory' existe pero no es una carpeta.");
        }
    } else {
        // No existe, se crea
        fs::create_directories(dir);
    }

    std::ofstream file(dump_path_txt);
    if (!file.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo de texto: " + dump_path_txt);
    }

    for (uint32_t word : data) {
        file << std::hex << std::setw(8) << std::setfill('0') << word << "\n";
    }

    file.close();
}


void SharedMemory::write_shared_memory_lines(const std::string& hex_128bits, size_t address) {
    if (hex_128bits.length() != 32) {
        std::cerr << "[SharedMemory] Error: se requieren exactamente 32 caracteres hexadecimales.\n";
        return;
    }

    if (address % 4 != 0) {
        std::cerr << "[SharedMemory] Error: la dirección debe estar alineada a 128 bits (múltiplo de 4).\n";
        return;
    }

    // Leer el archivo completo
    std::ifstream infile(dump_path_txt);
    if (!infile) {
        std::cerr << "[SharedMemory] Error: no se pudo abrir shared_memory.txt.\n";
        return;
    }

    std::vector<std::string> file_lines;
    std::string line;
    while (std::getline(infile, line)) {
        file_lines.push_back(line);
    }
    infile.close();

    if (address + 4 > file_lines.size()) {
        std::cerr << "[SharedMemory] Error: escritura fuera del rango del archivo.\n";
        return;
    }

    // Dividir en 4 segmentos de 8 caracteres (cada uno 32 bits)
    for (int i = 0; i < 4; ++i) {
        std::string word_hex = hex_128bits.substr(i * 8, 8);
        file_lines[address + i] = word_hex;
    }

    // Escribir todo el archivo de nuevo
    std::ofstream outfile(dump_path_txt);
    if (!outfile) {
        std::cerr << "[SharedMemory] Error: no se pudo abrir el archivo para escritura.\n";
        return;
    }

    for (const auto& l : file_lines) {
        outfile << l << "\n";
    }

    std::cout << "[SharedMemory] Línea de caché (128 bits) escrita correctamente en dirección " << address << ".\n";
}


std::vector<std::vector<std::string>> SharedMemory::read_shared_memory(size_t address, size_t size_bytes) {
    std::vector<std::vector<std::string>> result;

    // Abrir archivo directamente
    std::ifstream infile(dump_path_txt);
    if (!infile) {
        std::cerr << "[SharedMemory] Error: no se pudo abrir shared_memory.txt.\n";
        return result;
    }

    // Leer líneas del archivo
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // Calcular cantidad de líneas necesarias (cada línea = 4 bytes)
    size_t lines_to_read = static_cast<size_t>(std::ceil(size_bytes / 4.0));
    size_t blocks_to_read = (lines_to_read + 3) / 4; // bloques de 4 líneas (128 bits)

    for (size_t i = 0; i < blocks_to_read; ++i) {
        std::vector<std::string> block;
        for (size_t j = 0; j < 4; ++j) {
            size_t index = address + i * 4 + j;
            if (index < lines.size()) {
                block.push_back(lines[index]);
            } else {
                block.push_back("00000000");
            }
        }
        result.push_back(block);
    }

    return result;
}