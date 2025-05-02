#include "../include/System.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

/* ---------------------------------------- Constructor ---------------------------------------- */

System::System(int num_pes, ArbitScheme scheme)
    : total_pes_(num_pes), scheme_(scheme) {
    pes_.reserve(total_pes_);
    std::cout << "[System] Created with " << total_pes_ << " PEs and scheme "
              << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY") << "\n";   
}

/* --------------------------------------------------------------------------------------------- */

/* -------------------------------------- Initialization --------------------------------------- */

void System::initialize() {

    std::cout << "\n[System] Initializing Interconnect...\n";
    initialize_interconnect();
    
    std::cout << "\n[System] Initializing " << total_pes_ << " PEs...\n";
    initialize_pes();

    std::cout << "\n[System] Setting up Local Cache for every PE...\n";
    initialize_caches();

    // TODO: estas inicializaciones
    std::cout << "\n[System] Setting up Shared Memory...\n";

    std::cout << "\n[System] Getting simulation times...\n";

    std::cout << "\n[System] Setting up Statistics Unit...\n";

    std::cout << "\n[System] Initialization complete.\n";
}

void System::initialize_interconnect() {
    interconnect_ = std::make_unique<Interconnect>(total_pes_, scheme_);
}

void System::initialize_pes() {
    std::cout << "[System] Getting PEs' QoS from config/qos.txt...\n";
    // Mapa id -> QoS
    std::unordered_map<int, uint8_t> qos_map;
    std::ifstream infile("config/qos.txt");
    if (!infile) {
        std::cerr << "[System] Warning: could not open config/qos.txt, using default QoS=0 for all PEs\n";
    } else {
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            int id; char colon; std::string hexval;
            if (!(iss >> id >> colon >> hexval)) continue;
            try {
                uint8_t val = static_cast<uint8_t>(
                    std::stoul(hexval, nullptr, 16)
                );
                qos_map[id] = val;
            } catch (...) {
                std::cerr << "[System] Warning: invalid QoS entry: " << line << "\n";
            }
        }
    }

    // Instancia los PEs con QoS leído o 0 por defecto
    pes_.clear();
    for (int i = 0; i < total_pes_; ++i) {
        uint8_t qos = qos_map.count(i) ? qos_map[i] : 0;
        pes_.emplace_back(i, qos);
    }
}

void System::initialize_caches() {
    std::cout << "[System] Initializing local caches for " 
              << total_pes_ << " PEs...\n";

    caches_.clear();
    caches_.reserve(total_pes_);
    for (int i = 0; i < total_pes_; ++i) {
        caches_.emplace_back(i);  // construye un LocalCache vacío
        std::cout << "[System]  Cache " << i << " instantiated.\n";
    }

    std::cout << "[System] All local caches initialized.\n";
}

/* --------------------------------------------------------------------------------------------- */

/* ----------------------------------------- Execution ----------------------------------------- */

void System::run() {

    // **Depuración**: imprime cada PE y su QoS
    debug_print();

    const int cycles = 10; // Placeholder value
    for (int cycle = 0; cycle < cycles; ++cycle) {
        std::cout << "[Sim] Cycle " << (cycle + 1) << " executed.\n";
    }

    std::cout << "[System] Simulation finished.\n";
    std::cout << "[System] Saving results...\n";
    std::cout << "[System] Results saved in file.\n";
}

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Statistics ----------------------------------------- */

void System::report_statistics() const {
    std::cout << "\n[System] Reporting statistics for " << total_pes_
              << " PEs...\n";
    // TODO: Integrar con StatisticsUnit
    std::cout << "[System] (Placeholder)\n";
}

/* --------------------------------------------------------------------------------------------- */

/* ------------------------------------------- Debug ------------------------------------------- */

void System::debug_print() const {
    std::cout << "[System] Debug: General system state...";
    // Imprimir información de PEs
    std::cout << "\n[System] PEs and their QoS values:";
    for (const auto& pe : pes_) {
        std::cout << "  PE " << pe.get_id()
                  << " -> QoS=" << static_cast<int>(pe.get_qos()) << "";
    }
    // Llamar al debug del interconnect si existe
    if (interconnect_) {
        std::cout << "\n[System] Interconnect state:";
        interconnect_->debug_print();
    } else {
        std::cout << "\n[System] Interconnect not initialized.";
    }
}

/* --------------------------------------------------------------------------------------------- */
