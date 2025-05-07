#include "../include/Instruction_Generator.h"
#include <fstream>
#include <iostream>
#include <filesystem>

InstructionGenerator::InstructionGenerator(int num_pes, int instructions_per_file)
    : num_pes_(num_pes), instructions_per_file_(instructions_per_file), rng_(rd_()) {}

void InstructionGenerator::generate() const {

    const std::string dir = "config/assemblers";
    std::filesystem::create_directory(dir);

    for (int pe = 0; pe < num_pes_; ++pe) {
        std::string filename = dir + "/pe_" + std::to_string(pe) + ".txt";
        std::ofstream outfile(filename);
        if (!outfile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << "\n";
            continue;
        }

        for (int instr = 0; instr < instructions_per_file_; ++instr) {
            outfile << generate_instruction(pe) << "\n";
        }

        outfile.close();
        std::cout << "\n[Instruction Generator] Created " << filename << "\n";
    }
}

std::string InstructionGenerator::generate_instruction(int pe_id) const {
    std::uniform_int_distribution<int> instr_selector(0, 2);
    int instr_type = instr_selector(rng_);

    std::uniform_int_distribution<int> addr_dist(0, (MEM_SIZE - 1) / 4);
    std::uniform_int_distribution<int> cache_line_dist(0, CACHE_LINES - 1);
    
    std::uniform_int_distribution<int> size_dist(1, 64);
    std::uniform_int_distribution<int> qos_dist(0, MAX_QOS);

    std::string instruction;

    switch (instr_type) {
        case 0: { // WRITE_MEM
            int addr = addr_dist(rng_) * 4;
            int start_cache_line = cache_line_dist(rng_);
            std::uniform_int_distribution<int> num_cache_lines_dist(1, CACHE_LINES - start_cache_line);
            int num_cache_lines = num_cache_lines_dist(rng_);
            int qos = qos_dist(rng_);
            instruction = "WRITE_MEM " + std::to_string(pe_id) + ", " +
                          std::to_string(addr) + ", " +
                          std::to_string(num_cache_lines) + ", " +
                          std::to_string(start_cache_line) + ", " +
                          std::to_string(qos);
            break;
        }
        case 1: { // READ_MEM
            int addr = addr_dist(rng_) * 4;
            int size = size_dist(rng_) * 4;
            int qos = qos_dist(rng_);
            instruction = "READ_MEM " + std::to_string(pe_id) + ", " +
                          std::to_string(addr) + ", " +
                          std::to_string(size) + ", " +
                          std::to_string(qos);
            break;
        }
        case 2: { // BROADCAST_INVALIDATE
            int cache_line = cache_line_dist(rng_);
            int qos = qos_dist(rng_);
            instruction = "BROADCAST_INVALIDATE " + std::to_string(pe_id) + ", " +
                          std::to_string(cache_line) + ", " +
                          std::to_string(qos);
            break;
        }
    }

    return instruction;
}