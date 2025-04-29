#ifndef INSTRUCTION_MEMORY_H
#define INSTRUCTION_MEMORY_H

#include <string>
#include <vector>
#include <cstdint>

class InstructionMemory {
public:
    // Carga las instrucciones desde un archivo binario
    void load_from_binary(const std::string& filename);

    // Obtiene una instrucción específica (como vector de 6 bytes)
    std::vector<uint8_t> get_instruction(size_t index) const;

    // Devuelve la cantidad total de instrucciones almacenadas
    size_t size() const;

    // Imprime todas las instrucciones cargadas en hexadecimal (para debug)
    void print_memory() const;

private:
    static constexpr size_t INSTRUCTION_SIZE = 6; // 43 bits → se almacenan como 6 bytes (48 bits)
    std::vector<uint8_t> memory_;
};

#endif // INSTRUCTION_MEMORY_H
