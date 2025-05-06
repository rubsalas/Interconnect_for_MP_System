#include "../../include/components/Instruction_Memory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <bitset>
#include <filesystem>

InstructionMemory::InstructionMemory(int pe_id)
    : pe_id_(pe_id) {
        std::cout << "\n[IM] Created Instruction Memory for PE " << pe_id << "\n";
        // Guarda el directorio y el filename de donde obtener las instrucciones
        binary_path = "config/binaries/pe_" + std::to_string(pe_id_) + ".bin";
}

void InstructionMemory::initialize() {
    std::cout << "\n[IM PE" << pe_id_ << "] Loading instructions from '"
              << binary_path << "'...\n";

    // 1) Cargar desde archivo
    try {
        load_from_file(binary_path);
    } catch (const std::exception& e) {
        std::cerr << "[IM PE" << pe_id_ << "] Error cargando archivo: "
                  << e.what() << "\n";
        return;
    }

    // 2) Mostrar por consola
    std::cout << "[IM PE" << pe_id_ << "] Instrucciones cargadas (" 
              << size() << "):\n";
    for (size_t i = 0; i < size(); ++i) {
        uint64_t instr = fetch_instruction(i);
        std::cout << "  [" << i << "] 0x" 
                  << std::hex << instr 
                  << " (bin: " << std::bitset<64>(instr) << ")\n";
    }
    std::cout << std::dec; // restablecer flujo a decimal

    // 3) Volcar a fichero
    const std::string dir = "config/instruction_memories";
    std::filesystem::create_directory(dir);

    std::string dump_name = dir + "/dump_memoria_pe_" + std::to_string(pe_id_) + ".txt";
    try {
        dump_to_file(dump_name);
        std::cout << "[IM PE" << pe_id_ << "] Contenido exportado a " 
                  << dump_name << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[IM PE" << pe_id_ << "] Error volcando memoria: "
                  << e.what() << "\n";
    }
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

/* ---------------------------------------- Testing -------------------------------------------- */

void InstructionMemory::print_instructions() const {
    // 1) Cabecera con la cantidad de instrucciones
    std::cout << "[InstructionMemory] Printing "
              << instructions.size()
              << " instructions:\n";

    // 2) Recorremos cada instrucción por índice
    for (size_t idx = 0; idx < instructions.size(); ++idx) {
        uint64_t instr = instructions[idx];

        // 3) Imprimimos:
        //    - Índice en corchetes
        //    - Valor en hexadecimal (std::hex / std::dec)
        //    - Binario de 64 bits (std::bitset<64>)
        std::cout << "  [" << idx << "] "
                  << "0x" << std::hex << instr << std::dec
                  << " (bin: " << std::bitset<64>(instr) << ")\n";
    }

    // 4) Línea en blanco al final para separar visualmente
    std::cout << std::endl;
}

/* --------------------------------------------------------------------------------------------- */
