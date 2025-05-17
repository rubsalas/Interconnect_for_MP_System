#ifndef INSTRUCTION_GENERATOR_H
#define INSTRUCTION_GENERATOR_H

#include <string>
#include <random>

class InstructionGenerator {
public:
    InstructionGenerator(int num_pes, int instructions_per_file = 30);
    void generate() const;

private:
    static constexpr int MEM_SIZE = 4096;
    static constexpr int CACHE_LINES = 128;
    static constexpr int MAX_QOS = 3;

    int num_pes_;
    int instructions_per_file_;
    std::random_device rd_;
    mutable std::mt19937 rng_;

    std::string generate_instruction(int pe_id) const;
};

#endif // INSTRUCTION_GENERATOR_H