#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
#include <cstdlib>

// Rango de campos según las restricciones
constexpr int MEM_SIZE = 4096;                     // Memoria alineada a 4 bytes
constexpr int CACHE_LINES = 128;                   // Cache por PE
constexpr int MAX_QOS = 3;                         // QoS de 0 a 3
constexpr int INSTRUCTIONS_PER_FILE = 10;          // Instrucciones por archivo (fijo)

std::string generate_instruction(int pe_id, std::mt19937& rng) {
    std::uniform_int_distribution<int> instr_selector(0, 2);
    int instr_type = instr_selector(rng);

    std::uniform_int_distribution<int> addr_dist(0, (MEM_SIZE - 1) / 4); // direcciones alineadas a 4
    std::uniform_int_distribution<int> cache_line_dist(0, CACHE_LINES - 1);
    std::uniform_int_distribution<int> num_cache_lines_dist(1, CACHE_LINES);
    std::uniform_int_distribution<int> size_dist(1, 64);  // SIZE de 4 a 256 bytes (multiplicado por 4)
    std::uniform_int_distribution<int> qos_dist(0, MAX_QOS);

    std::string instruction;

    switch (instr_type) {
        case 0: { // WRITE_MEM
            int addr = addr_dist(rng) * 4;
            int num_cache_lines = num_cache_lines_dist(rng);
            int start_cache_line = cache_line_dist(rng);
            int qos = qos_dist(rng);
            instruction = "WRITE_MEM " + std::to_string(pe_id) + ", " +
                          std::to_string(addr) + ", " +
                          std::to_string(num_cache_lines) + ", " +
                          std::to_string(start_cache_line) + ", " +
                          std::to_string(qos);
            break;
        }
        case 1: { // READ_MEM
            int addr = addr_dist(rng) * 4;
            int size = size_dist(rng) * 4;
            int qos = qos_dist(rng);
            instruction = "READ_MEM " + std::to_string(pe_id) + ", " +
                          std::to_string(addr) + ", " +
                          std::to_string(size) + ", " +
                          std::to_string(qos);
            break;
        }
        case 2: { // BROADCAST_INVALIDATE
            int cache_line = cache_line_dist(rng);
            int qos = qos_dist(rng);
            instruction = "BROADCAST_INVALIDATE " + std::to_string(pe_id) + ", " +
                          std::to_string(cache_line) + ", " +
                          std::to_string(qos);
            break;
        }
    }

    return instruction;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_PEs>\n";
        return EXIT_FAILURE;
    }

    int num_pes = std::atoi(argv[1]);
    if (num_pes <= 0) {
        std::cerr << "Error: Number of PEs must be positive.\n";
        return EXIT_FAILURE;
    }

    std::random_device rd;
    std::mt19937 rng(rd());

    for (int pe = 0; pe < num_pes; ++pe) {
        std::string filename = "pe_" + std::to_string(pe) + ".txt";
        std::ofstream outfile(filename);
        if (!outfile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << "\n";
            continue;
        }

        for (int instr = 0; instr < INSTRUCTIONS_PER_FILE; ++instr) {
            outfile << generate_instruction(pe, rng) << "\n";
        }

        outfile.close();
        std::cout << "Generated " << filename << "\n";
    }

    return EXIT_SUCCESS;
}
