#include "../include/Compiler.h"
#include <filesystem>
#include <bitset>
#include <sstream>
#include <regex>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

// Tamaños de campo en bits
constexpr int ADDR_SIZE = 16;
constexpr int CACHE_LINE_SIZE = 8;
constexpr int QOS_SIZE = 4;
constexpr int SRC_SIZE = 5;

// Límites de validación
constexpr int MAX_ADDR = 4096 * 4;
constexpr int MAX_CACHE_LINE = 512;
constexpr int MAX_QOS = 15;
constexpr int MAX_SRC = 31;

// ISA con sus respectivos opcodes de 2 bits
const std::map<std::string, std::string> isa = {
    {"WRITE_MEM", "00"},
    {"READ_MEM", "01"},
    {"BROADCAST_INVALIDATE", "10"}
};

std::string Compiler::to_bin(int value, int bits) {
    return std::bitset<64>(value).to_string().substr(64 - bits);
}

std::string Compiler::validate_src(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num < 0 || num > MAX_SRC) throw std::invalid_argument("SRC inválido: " + value);
    return to_bin(num, SRC_SIZE);
}

std::string Compiler::validate_addr(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num % 4 != 0) throw std::invalid_argument("Dirección no alineada a 4 bytes: " + value);
    if (num < 0 || num >= MAX_ADDR) throw std::invalid_argument("Dirección fuera de rango: " + value);
    return to_bin(num, ADDR_SIZE);
}

std::string Compiler::validate_cache_line(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num < 0 || num >= MAX_CACHE_LINE) throw std::invalid_argument("Línea de caché fuera de rango: " + value);
    return to_bin(num, CACHE_LINE_SIZE);
}

std::string Compiler::validate_qos(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num < 0 || num > MAX_QOS) throw std::invalid_argument("QoS fuera de rango: " + value);
    return to_bin(num, QOS_SIZE);
}

std::vector<std::vector<std::string>> Compiler::clean_instructions(std::ifstream& file) {
    std::vector<std::vector<std::string>> instructions;
    std::string line;
    std::regex re("[\\s,]+");

    while (std::getline(file, line)) {
        line = line.substr(0, line.find(';')); // strip comments
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> parts;
        while (std::getline(ss, token, ' ')) {
            if (!token.empty()) {
                std::sregex_token_iterator it(token.begin(), token.end(), re, -1), end;
                for (; it != end; ++it) {
                    if (!it->str().empty()) parts.push_back(it->str());
                }
            }
        }
        if (!parts.empty()) instructions.push_back(parts);
    }
    return instructions;
}

std::vector<std::string> Compiler::get_binary(const std::vector<std::vector<std::string>>& instructions) {
    std::vector<std::string> binary_instr;
    for (const auto& instr : instructions) {
        const std::string& mnemonic = instr[0];
        auto it = isa.find(mnemonic);
        if (it == isa.end()) throw std::invalid_argument("Instrucción no válida: " + mnemonic);
        const std::string& opcode = it->second;

        if (mnemonic == "WRITE_MEM") {
            std::string src       = validate_src(instr[1]);
            std::string addr      = validate_addr(instr[2]);
            std::string num_cl    = validate_cache_line(instr[3]);
            std::string start_cl  = validate_cache_line(instr[4]);
            std::string qos       = validate_qos(instr[5]);
            binary_instr.push_back(opcode + src + addr + num_cl + start_cl + qos);
        }
        else if (mnemonic == "READ_MEM") {
            std::string src       = validate_src(instr[1]);
            std::string addr      = validate_addr(instr[2]);
            std::string size      = validate_cache_line(instr[3]);
            std::string relleno   = to_bin(0, CACHE_LINE_SIZE);
            std::string qos       = validate_qos(instr[4]);
            binary_instr.push_back(opcode + src + addr + size + relleno + qos);
        }
        else if (mnemonic == "BROADCAST_INVALIDATE") {
            std::string src       = validate_src(instr[1]);
            std::string relleno1  = to_bin(0, CACHE_LINE_SIZE);
            std::string cl        = validate_cache_line(instr[2]);
            std::string relleno2  = to_bin(0, ADDR_SIZE);
            std::string qos       = validate_qos(instr[3]);
            binary_instr.push_back(opcode + src + relleno1 + cl + relleno2 + qos);
        }
    }
    return binary_instr;
}

void Compiler::compile_directory(const std::string& input_dir,
                                 const std::string& output_dir) const {
    namespace fs = std::filesystem;
    try {
        fs::create_directories(output_dir);
    } catch (const std::exception& e) {
        std::cerr << "[Compiler] Error creating directory " << output_dir
                  << ": " << e.what() << "\n";
        return;
    }

    for (auto const& entry : fs::directory_iterator(input_dir)) {
        if (!entry.is_regular_file()) continue;
        const auto in_path  = entry.path().string();
        const auto stem     = entry.path().stem().string();
        const auto out_path = output_dir + "/" + stem + ".bin";

        std::ifstream input(in_path);
        if (!input) {
            std::cerr << "[Compiler] Cannot open " << in_path << "\n";
            continue;
        }
        auto instr     = clean_instructions(input);
        auto bin_instr = get_binary(instr);

        std::ofstream output(out_path);
        if (!output) {
            std::cerr << "[Compiler] Cannot create " << out_path << "\n";
            continue;
        }
        for (auto& line : bin_instr) output << line << "\n";
        std::cout << "[Compiler] " << in_path << " -> " << out_path << "\n";
    }
}
