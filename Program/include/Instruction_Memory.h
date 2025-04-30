#ifndef INSTRUCTION_MEMORY_H
#define INSTRUCTION_MEMORY_H

#include <vector>
#include <cstdint>
#include <string>

class InstructionMemory {
private:
    std::vector<uint64_t> instructions;

public:
    void load(const std::vector<uint64_t>& instrs);
    void load_from_file(const std::string& filename);
    uint64_t fetch_instruction(size_t address) const;
    size_t size() const;
    void dump_to_file(const std::string& output_filename) const;

private:
    uint64_t parse_instruction(const std::string& text);
};

#endif // INSTRUCTION_MEMORY_H
