#include "../include/System.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "../include/Message.h"
#include <bitset>


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

    std::cout << "\n[System] Setting up Shared Memory...\n";
    initialize_shared_memory();

    // TODO: estas inicializaciones
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

void System::initialize_shared_memory() {
    shared_memory_ = std::make_unique<SharedMemory>();
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

/* ---------------------------------------- Testing -------------------------------------------- */

/**
 * @brief Carga instrucciones binarias desde un archivo, las decodifica a objetos Message
 *        y los imprime en consola y en un archivo de salida.
 *
 * Esta función interpreta cada línea del archivo como una instrucción de 64 bits, 
 * donde el bit más significativo es el bit 63. Se ignoran los bits 63–43. A partir
 * de los bits restantes, se extraen los campos según el tipo de instrucción (WRITE_MEM,
 * READ_MEM, BROADCAST_INVALIDATE) y se construyen objetos Message. 
 * 
 * Cada mensaje decodificado se imprime en consola (en formato compacto y detallado)
 * y se guarda en el archivo mensajes_generados.txt.
 *
 * @param file_path Ruta al archivo que contiene instrucciones binarias en texto plano.
 * 
 * @note Se espera que cada línea del archivo contenga exactamente 64 caracteres ('0' o '1').
 * 
 * @warning Las líneas con menos de 64 bits serán ignoradas.
 */

void System::system_test_G(const std::string& file_path) {
	std::cout << "\n[TEST] Starting System Test G...\n";
    std::ifstream infile(file_path);
    std::ofstream outfile("mensajes_generados.txt");

    std::string line;
    int mensajes_generados = 0;

    if (!infile) {
        std::cerr << "[System] Error: could not open " << file_path << "\n";
        return;
    }

    outfile << "[System] Decoded messages from: " << file_path << "\n";

    while (std::getline(infile, line)) {
        if (line.length() < 64) {
            std::cerr << "[Warning] Ignoring line with length < 64: " << line << "\n";
            continue;
        }

        std::string opcode = line.substr(21, 2);  // Bits 42–41

        Message msg(Operation::UNDEFINED);

        if (opcode == "00") { // WRITE_MEM
            msg = Message(
                Operation::WRITE_MEM,
                std::bitset<5>(line.substr(23, 5)).to_ulong(),     // src: bits 40–36
                0,
                std::bitset<16>(line.substr(28, 16)).to_ulong(),   // address: bits 35–20
                std::bitset<4>(line.substr(60, 4)).to_ulong(),     // qos: bits 3–0
                0,
                std::bitset<8>(line.substr(44, 8)).to_ulong(),     // num_cache_lines: bits 19–12
                std::bitset<8>(line.substr(52, 8)).to_ulong(),     // start_cache_line: bits 11–4
                0, 0, {}
            );

        } else if (opcode == "01") { // READ_MEM
            msg = Message(
                Operation::READ_MEM,
                std::bitset<5>(line.substr(23, 5)).to_ulong(),
                0,
                std::bitset<16>(line.substr(28, 16)).to_ulong(),
                std::bitset<4>(line.substr(60, 4)).to_ulong(),
                std::bitset<8>(line.substr(44, 8)).to_ulong(),
                0, 0, 0, 0, {}
            );

        } else if (opcode == "10") { // BROADCAST_INVALIDATE
            msg = Message(
                Operation::BROADCAST_INVALIDATE,
                std::bitset<5>(line.substr(23, 5)).to_ulong(),
                0,
                0,
                std::bitset<4>(line.substr(60, 4)).to_ulong(),
                0, 0, 0,
                std::bitset<8>(line.substr(36, 8)).to_ulong(),     // cache_lines (bits 27–20)
                0, {}
            );

        } else {
            std::cerr << "[Warning] Opcode no reconocido: " << opcode << "\n";
            continue;
        }

        // Imprimir en consola y escribir en archivo
        std::cout << "[DEBUG] Mensaje creado: " << msg.to_string() << std::endl;
        outfile << msg.to_string() << "\n";
        mensajes_generados++;
    }

    outfile << "[System] Message decoding complete.\n";
    outfile << "[System] Total messages decoded: " << mensajes_generados << "\n";
    std::cout << "[System] Total messages decoded: " << mensajes_generados << "\n";

    outfile.close();
}
    


void System::system_test_R() {
	std::cout << "\n[TEST] Starting System Test R...\n";
	// TODO
}

/* --------------------------------------------------------------------------------------------- */
