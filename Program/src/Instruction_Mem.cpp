#include "../include/Instruction_Mem.h"
#include <fstream>
#include <iostream>
#include <iomanip>

void InstructionMemory::load_from_binary(const std::string& filename) {
    std::ifstream infile(filename, std::ios::binary);
    if (!infile.is_open()) {
        throw std::runtime_error("Error: Could not open binary instruction file: " + filename);
    }

    infile.seekg(0, std::ios::end);
    std::streamsize file_size = infile.tellg();
    infile.seekg(0, std::ios::beg);

    if (file_size % INSTRUCTION_SIZE != 0) {
        throw std::runtime_error("Error: File size is not aligned with 6-byte instructions.");
    }

    memory_.resize(file_size);
    infile.read(reinterpret_cast<char*>(memory_.data()), file_size);
    infile.close();
}

std::vector<uint8_t> InstructionMemory::get_instruction(size_t index) const {
    if (index * INSTRUCTION_SIZE >= memory_.size()) {
        throw std::out_of_range("Error: Instruction index out of range.");
    }

    return std::vector<uint8_t>(memory_.begin() + index * INSTRUCTION_SIZE,
                                memory_.begin() + (index + 1) * INSTRUCTION_SIZE);
}

size_t InstructionMemory::size() const {
    return memory_.size() / INSTRUCTION_SIZE;
}

void InstructionMemory::print_memory() const {
    for (size_t i = 0; i < size(); ++i) {
        auto instr = get_instruction(i);
        std::cout << "Instruction " << i << ": ";
        for (auto byte : instr) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << "\n";
    }
}
