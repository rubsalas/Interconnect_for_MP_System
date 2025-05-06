#pragma once

#include <cstdint>
#include <string>
#include "Instruction_Memory.h"
#include "../Message.h"

/**
 * @enum PEState
 * @brief Estados internos de un PE durante la simulación.
 */
enum class PEState {
    IDLE,       /**< Aún no ha comenzado, o ya terminó y está inactivo. */
    RUNNING,    /**< Ejecutando instrucciones normalmente (fetch/decode/issue). */
    STALLED,    /**< Detenido esperando respuesta de memoria/coherencia. */
    FINISHED    /**< Ha encontrado la instrucción END y ya no ejecutará más. */
};

/**
 * @class PE
 * @brief Processing Element con QoS, contador de programa, instrucción actual
 *        y memoria de instrucciones.
 */
class PE {
public:
    InstructionMemory   instruction_memory_;    /**< Memoria de instrucciones. */

    /**
     * @brief Construye un PE con identificador y QoS.
     * @param id  Identificador único del PE.
     * @param qos Calidad de servicio (0x00–0xFF).
     */
    PE(int id, uint8_t qos);

/* ----------------------------------- Getters & Setters --------------------------------------- */

    /** @brief Devuelve el identificador del PE. */
    int get_id() const;
    
    /** @brief Devuelve el valor de QoS asignado. */
    uint8_t get_qos() const;

    /** @brief Consulta el estado actual del PE. */
    PEState get_state() const;
    /** @brief Cambia el estado del PE. */
    void set_state(PEState s);

    /** @brief Devuelve el valor del Program Counter. */
    uint64_t get_pc() const;
    /** @brief Establece el valor del Program Counter. */
    void set_pc(uint64_t pc);

    /** @brief Devuelve la instrucción de 64 bits actual. */
    uint64_t get_actual_instruction() const;
    /** @brief Establece la instrucción de 64 bits actual. */
    void set_actual_instruction(uint64_t instr);

    /**
     * @brief Obtiene el mensaje actual de depuración.
     * @return Referencia constante al Message guardado internamente.
     */
    const Message& get_actual_message() const;
    /**
     * @brief Establece el mensaje actual de depuración.
     * @param msg Mensaje (Message) que se desea almacenar en este PE.
     */
    void set_actual_message(const Message& msg);

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

    /**
     * @brief Imprime en consola el estado interno del PE para depuración.
     *
     * Muestra su ID, QoS, valor del PC y la instrucción actual
     * (como hexadecimal y binario).
     */
    void debug_print() const;

    /**
     * @brief Obtiene el nombre literal del estado actual.
     * @return Cadena estática con el nombre del estado.
     */
    const char* state_to_string() const;

/* --------------------------------------------------------------------------------------------- */

private:
    int                 id_;                    /**< ID del PE. */
    uint8_t             qos_;                   /**< Calidad de servicio. */
    PEState             state_{PEState::IDLE};    /**< Estado del PE */ /* NEW */
    uint64_t            pc_;                    /**< Program Counter. */
    uint64_t            actual_instruction_;    /**< Instrucción de 64 bits. */
    Message             actual_message_;        /**< Mensaje de prueba para debug. */
};
