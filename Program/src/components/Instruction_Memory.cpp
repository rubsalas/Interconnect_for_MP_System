#include "../include/components/Instruction_Memory.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>  // Para std::setw si quisieras hex
#include <bitset>   // Para escribir binarios

void InstructionMemory::load(const std::vector<uint64_t>& instrs) {
    instructions = instrs;
}

void InstructionMemory::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: No se pudo abrir el archivo " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            uint64_t value = parse_instruction(line);
            instructions.push_back(value);
        }
    }
    file.close();
}

uint64_t InstructionMemory::fetch_instruction(size_t address) const {
    if (address >= instructions.size()) {
        throw std::out_of_range("Error: Dirección fuera de rango en la memoria de instrucciones.");
    }
    return instructions[address];
}

size_t InstructionMemory::size() const {
    return instructions.size();
}

uint64_t InstructionMemory::parse_instruction(const std::string& text) {
    uint64_t value = 0;

    if (text.find("0x") == 0 || text.find("0X") == 0) {
        value = std::stoull(text, nullptr, 16);
    } else {
        if (text.length() > 64) {
            throw std::invalid_argument("Error: Instrucción binaria supera los 64 bits.");
        }
        for (char c : text) {
            if (c != '0' && c != '1') {
                throw std::invalid_argument("Error: Caracter inválido en instrucción binaria: " + text);
            }
            value = (value << 1) | (c - '0');
        }
    }

    if (value >= (1ULL << 43)) {
        throw std::overflow_error("Error: Instrucción supera los 43 bits permitidos.");
    }

    return value;
}

void InstructionMemory::dump_to_file(const std::string& output_filename) const {
    std::ofstream out(output_filename);
    if (!out.is_open()) {
        throw std::runtime_error("Error: No se pudo crear el archivo de volcado: " + output_filename);
    }

    for (const auto& instr : instructions) {
        // Escribe la instrucción como 64 bits en binario
        out << std::bitset<64>(instr) << "\n";
    }

    out.close();
}
