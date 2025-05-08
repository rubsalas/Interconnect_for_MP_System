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
        dump_path_txt = "config/shared_memory/shared_memory.txt";
        dump_path_bin = "config/shared_memory/shared_memory.bin";

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
    fs::path dir = "config/shared_memory";
    if (fs::exists(dir)) {
        if (!fs::is_directory(dir)) {
            throw std::runtime_error("Error: 'shared_memory' existe pero no es una carpeta.");
        }
    } else {
        // No existe, se crea
        fs::create_directories(dir);
    }

    std::ofstream out(dump_path_txt);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo de texto: " + dump_path_txt);
    }

    for (const auto& elem : data) {
        out << std::bitset<32>(elem) << "\n"; // Escribir cada valor como string binario de 32 bits
    }

    out.close();
}
