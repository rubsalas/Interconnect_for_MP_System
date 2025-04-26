// compiler.h
#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <fstream>

std::string to_bin(int value, int bits);
std::string validate_src(const std::string& value);
std::string validate_addr(const std::string& value);
std::string validate_cache_line(const std::string& value);
std::string validate_qos(const std::string& value);
std::vector<std::vector<std::string>> clean_instructions(std::ifstream& file);
std::vector<std::string> get_binary(const std::vector<std::vector<std::string>>& instructions);

#endif // COMPILER_H
