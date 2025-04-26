// compiler.cpp
#include "../include/Compiler.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <map>
#include <bitset>
#include <stdexcept>

/// Tamaños de campo en bits
constexpr int ADDR_SIZE = 16;
constexpr int CACHE_LINE_SIZE = 8;
constexpr int QOS_SIZE = 4;
constexpr int SRC_SIZE = 5;

/// Límites de validación
constexpr int MAX_ADDR = 4096 * 4;
constexpr int MAX_CACHE_LINE = 512;
constexpr int MAX_QOS = 15;
constexpr int MAX_SRC = 31;

/// ISA con sus respectivos opcodes de 2 bits
const std::map<std::string, std::string> isa = {
    {"WRITE_MEM", "00"},
    {"READ_MEM", "01"},
    {"BROADCAST_INVALIDATE", "10"}
};

/**
 * @brief Convierte un valor entero a una cadena binaria de longitud fija.
 * @param value Valor entero a convertir.
 * @param bits Número de bits deseado.
 * @return Cadena binaria.
 */
std::string to_bin(int value, int bits) {
    return std::bitset<64>(value).to_string().substr(64 - bits);
}

/**
 * @brief Valida y convierte el campo SRC a binario.
 * @param value Cadena con el valor a validar.
 * @return Cadena binaria de 5 bits.
 */
std::string validate_src(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num < 0 || num > MAX_SRC) throw std::invalid_argument("SRC inválido: " + value);
    return to_bin(num, SRC_SIZE);
}

/**
 * @brief Valida y convierte la dirección a binario.
 * @param value Cadena con el valor a validar.
 * @return Cadena binaria de 16 bits.
 */
std::string validate_addr(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num % 4 != 0) throw std::invalid_argument("Dirección no alineada a 4 bytes: " + value);
    if (num < 0 || num >= MAX_ADDR) throw std::invalid_argument("Dirección fuera de rango: " + value);
    return to_bin(num, ADDR_SIZE);
}

/**
 * @brief Valida y convierte un número de línea de caché a binario.
 * @param value Cadena con el valor a validar.
 * @return Cadena binaria de 8 bits.
 */
std::string validate_cache_line(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num < 0 || num >= MAX_CACHE_LINE) throw std::invalid_argument("Línea de caché fuera de rango: " + value);
    return to_bin(num, CACHE_LINE_SIZE);
}

/**
 * @brief Valida y convierte el valor de QoS a binario.
 * @param value Cadena con el valor a validar.
 * @return Cadena binaria de 4 bits.
 */
std::string validate_qos(const std::string& value) {
    int num = std::stoi(value, nullptr, 0);
    if (num < 0 || num > MAX_QOS) throw std::invalid_argument("QoS fuera de rango: " + value);
    return to_bin(num, QOS_SIZE);
}

/**
 * @brief Limpia y divide las instrucciones del archivo en componentes.
 * @param file Archivo de entrada con instrucciones.
 * @return Vector de vectores con las instrucciones tokenizadas.
 */
std::vector<std::vector<std::string>> clean_instructions(std::ifstream& file) {
    std::vector<std::vector<std::string>> instructions;
    std::string line;
    std::regex re("[\\s,]+");

    while (std::getline(file, line)) {
        line = line.substr(0, line.find(';'));
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

/**
 * @brief Convierte una lista de instrucciones en su representación binaria.
 * @param instructions Vector de instrucciones tokenizadas.
 * @return Vector de instrucciones codificadas en binario.
 */
std::vector<std::string> get_binary(const std::vector<std::vector<std::string>>& instructions) {
    std::vector<std::string> binary_instr;

    for (const auto& instr : instructions) {
        const std::string& mnemonic = instr[0];
        const std::string& opcode = isa.at(mnemonic);

        if (mnemonic == "WRITE_MEM") {
            std::string src = validate_src(instr[1]);
            std::string addr = validate_addr(instr[2]);
            std::string num_cl = validate_cache_line(instr[3]);
            std::string start_cl = validate_cache_line(instr[4]);
            std::string qos = validate_qos(instr[5]);
            binary_instr.push_back(opcode + src + addr + num_cl + start_cl + qos);

        } else if (mnemonic == "READ_MEM") {
            std::string src = validate_src(instr[1]);
            std::string addr = validate_addr(instr[2]);
            std::string size = validate_cache_line(instr[3]);
            std::string relleno = to_bin(0, 8);
            std::string qos = validate_qos(instr[4]);
            binary_instr.push_back(opcode + src + addr + size + relleno + qos);

        } else if (mnemonic == "BROADCAST_INVALIDATE") {
            std::string src = validate_src(instr[1]);
            std::string relleno1 = to_bin(0, 8);
            std::string cache_line = validate_cache_line(instr[2]);
            std::string relleno2 = to_bin(0, 16);
            std::string qos = validate_qos(instr[3]);
            binary_instr.push_back(opcode + src + relleno1 + cache_line + relleno2 + qos);

        } else {
            throw std::invalid_argument("Instrucción no válida: " + mnemonic);
        }
    }
    return binary_instr;
}
