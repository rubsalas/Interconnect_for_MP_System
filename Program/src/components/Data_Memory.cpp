#include "../include/components/Data_Memory.h"
#include <random>
#include <fstream>
#include <bitset>  // Para convertir a binario como string

DataMemory::DataMemory()
    : data(MEMORY_SIZE, 0)
{}

uint32_t DataMemory::load(size_t address) const {
    if (address >= MEMORY_SIZE) {
        throw std::out_of_range("Error: Dirección de memoria fuera de rango en load().");
    }
    return data[address];
}

void DataMemory::store(size_t address, uint32_t value) {
    if (address >= MEMORY_SIZE) {
        throw std::out_of_range("Error: Dirección de memoria fuera de rango en store().");
    }
    data[address] = value;
}

size_t DataMemory::size() const {
    return MEMORY_SIZE;
}

void DataMemory::fill_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> distrib(0, UINT32_MAX);

    for (auto& elem : data) {
        elem = distrib(gen);
    }
}

void DataMemory::dump_to_binary_file(const std::string& output_filename) const {
    std::ofstream out(output_filename, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo binario: " + output_filename);
    }
    out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint32_t));
    out.close();
}

void DataMemory::dump_to_text_file(const std::string& output_filename) const {
    std::ofstream out(output_filename);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo de texto: " + output_filename);
    }

    for (const auto& elem : data) {
        out << std::bitset<32>(elem) << "\n"; // Escribir cada valor como string binario de 32 bits
    }

    out.close();
}
