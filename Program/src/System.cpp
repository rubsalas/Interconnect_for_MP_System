#include "../include/System.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

System::System(int num_pes, ArbitScheme scheme)
    : total_pes_(num_pes), scheme_(scheme) {
    pes_.reserve(total_pes_);
    std::cout << "[System] Created with " << total_pes_ << " PEs and scheme "
              << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY") << "\n";
    // Instancia el Interconnect inmediatamente
    interconnect_ = std::make_unique<Interconnect>(total_pes_, scheme_);
}

void System::initialize() {
    
    std::cout << "[System] Initializing " << total_pes_ << " PEs...\n";
    initialize_pes(); // Funcion aparte que leerá el archivo

    // TODO: todas estas inicializaciones
    std::cout << "[System] Setting PEs' Instruction Memory...\n";
    std::cout << "[System] Setting up Caches...\n";
    std::cout << "[System] Setting up Shared Memory...\n";
    std::cout << "[System] Getting simulation times...\n";
    std::cout << "[System] Setting up Statistics Unit...\n";

    std::cout << "[System] Initialization complete.\n";
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

void System::report_statistics() const {
    std::cout << "\n[System] Reporting statistics for " << total_pes_
              << " PEs...\n";
    // TODO: Integrar con StatisticsUnit
    std::cout << "[System] (Placeholder)\n";
}

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
