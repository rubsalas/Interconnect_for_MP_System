#include <iostream>
#include <string>
#include <limits>

// Compile with: g++ -std=c++20 main.cpp -o interconnect_sim

constexpr int MAX_PES = 32;          // Upper limit for Processing Elements
int pe_count = 0;                    // Global variable to store the configured PE count

// Forward declarations
void show_menu();
void initialize_system();
void run_simulation();
void show_statistics();

int main() {
    bool running = true;
    int choice = -1;

    std::cout << "=== Interconnect for MP Systems Simulator ===\n";
    while (running) {
        show_menu();
        std::cout << "Select option: ";

        if (!(std::cin >> choice)) {
            // Clear invalid input and prompt again
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[Error] Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1:
                initialize_system();
                break;
            case 2:
                run_simulation();
                break;
            case 3:
                show_statistics();
                break;
            case 0:
                std::cout << "\nExiting program...\n";
                running = false;
                break;
            default:
                std::cout << "[Error] Option not recognized.\n";
        }
    }

    std::cout << "Program terminated.\n";
    return 0;
}

// ---------------- Helper Function Definitions ----------------

void show_menu() {
    std::cout << "\n--- Main Menu ---\n"
              << "1. Initialize System\n"
              << "2. Run Simulation\n"
              << "3. Show Statistics\n"
              << "0. Exit\n";
}

void initialize_system() {

    int requested_pes = 0;

    std::cout << "\n[Init] Enter number of Processing Elements (1-" << MAX_PES << "): ";

    // Keep prompting until a valid number is provided
    while (true) {
        if (!(std::cin >> requested_pes)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[Error] Invalid input. Please enter an integer between 1 and " << MAX_PES << ": ";
            continue;
        }

        if (requested_pes < 1 || requested_pes > MAX_PES) {
            std::cout << "[Error] Value out of range. Please enter a number between 1 and " << MAX_PES << ": ";
        } else {
            break; // Valid input
        }
    }

    pe_count = requested_pes;
    std::cout << "[Init] System configured with " << pe_count << " PEs.\n";

    std::cout << "[Init] Setting up PEs...\n";
    std::cout << "[Init] Getting PE's QoS...\n";
    std::cout << "[Init] Setting up Caches...\n";
    std::cout << "[Init] Setting up Interconnect...\n";
    std::cout << "[Init] Setting up Shared Memory...\n";
    std::cout << "[Init] Getting simulation times...\n";
    std::cout << "[Init] Compiling Instructions...\n";
    std::cout << "[Init] Setting PEs' Instructions Memory...\n";
    std::cout << "[Init] Setting up Statistics Unit...\n";

    // TODO: Initialization logic here
    std::cout << "[Init] Initialization complete.\n";
}

void run_simulation() {
    if (pe_count == 0) {
        std::cout << "\n[Sim] System not initialized. Please initialize first.\n";
        return;
    }

    std::cout << "\n[Sim] Starting simulation with " << pe_count << " PEs...\n";
    const int cycles = 10; // Placeholder value

    for (int cycle = 0; cycle < cycles; ++cycle) {
        // Placeholder logic for a simulation cycle
        std::cout << "[Sim] Cycle " << (cycle + 1) << " executed.\n";
    }

    std::cout << "[Sim] Simulation finished.\n";
    std::cout << "[Sim] Saving results...\n";
    std::cout << "[Sim] Results saved in file.\n";
}

void show_statistics() {
    if (pe_count == 0) {
        std::cout << "\n[Stats] No statistics available: system not initialized.\n";
        return;
    }

    std::cout << "\n[Stats] Placeholder statistics for " << pe_count << " PEs from results file.\n";
}
