#include <iostream>
#include <cstdlib>
#include "../include/components/PE.h" // Aquí se asume que convert_to_message está declarada aquí
#include "../include/components/Instruction_Memory.h" 
#include "../include/Message.h" 
#include <bitset>
#include <fstream>


// Declaración anticipada
Message convert_to_message(const InstructionMemory& instruction_memory_, int index);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <archivo_instrucciones> <index>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    int index = std::atoi(argv[2]);

    // Cargamos las instrucciones manualmente (sin usar initialize ni pe_id)
    InstructionMemory memoria(0); // Constructor requiere un ID, pero no se usará
    memoria.instructions.clear(); // Limpia cualquier instrucción previa

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        uint64_t instr = std::bitset<64>(line).to_ullong();
        memoria.instructions.push_back(instr);
    }

    try {
        Message msg = convert_to_message(memoria, index);

        std::cout << "Mensaje decodificado:\n";
        std::cout << msg.to_string() << std::endl;
        std::cout << static_cast<int>(msg.get_operation())  << std::endl;


    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}