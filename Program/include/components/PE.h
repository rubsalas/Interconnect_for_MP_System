#pragma once

#include <cstdint>
#include <string>
#include "Instruction_Memory.h"

/**
 * @class PE
 * @brief Processing Element con QoS, contador de programa, instrucción actual
 *        y memoria de instrucciones.
 */
class PE {
public:
    /**
     * @brief Construye un PE con identificador y QoS.
     * @param id  Identificador único del PE.
     * @param qos Calidad de servicio (0x00–0xFF).
     */
    PE(int id, uint8_t qos);

    /** @brief Devuelve el identificador del PE. */
    int get_id() const;
    /** @brief Devuelve el valor de QoS asignado. */
    uint8_t get_qos() const;

    /** @brief Devuelve el valor del Program Counter. */
    uint64_t get_pc() const;
    /** @brief Establece el valor del Program Counter. */
    void set_pc(uint64_t pc);

    /** @brief Devuelve la instrucción de 64 bits actual. */
    uint64_t get_actual_instruction() const;
    /** @brief Establece la instrucción de 64 bits actual. */
    void set_actual_instruction(uint64_t instr);

    /**
     * @brief Carga las instrucciones desde un archivo en la InstructionMemory.
     * @param filename Ruta al archivo de instrucciones.
     */
    void load_instructions(const std::string& filename);

private:
    int                 id_;                   /**< ID del PE. */
    uint8_t             qos_;                  /**< Calidad de servicio. */
    uint64_t            pc_;                   /**< Program Counter. */
    InstructionMemory   instruction_memory_;   /**< Memoria de instrucciones. */
    uint64_t            actual_instruction_;   /**< Instrucción de 64 bits. */
};
