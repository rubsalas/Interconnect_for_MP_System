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

enum class PEResponseState {
    READY,       /**< Sin petición activa, no espera nada. */
    WAITING,     /**< Puede haber enviado un mensaje o no, siempre espera una respuesta. */
    PROCESSING,  /**< Ha recibido respuesta y la está procesando. */
    COMPLETED    /**< Procesó la respuesta y está listo para avanzar. */
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

    /** @brief Incrementa el Program Counter en 1. */
    void pc_plus_4();

    /**
     * @brief Convierte una instrucción alojada en InstructionMemory en un Message.
     *
     * Recupera la cadena binaria de 64 bits almacenada en la posición
     * `instruction_index` de la InstructionMemory, la interpreta como un
     * entero de 64 bits, extrae los distintos campos (opcode, src, addr,
     * tamaño, líneas de caché, QoS, etc.) según el formato definido en la
     * especificación, y construye el Message correspondiente con la
     * operación adecuada (WRITE_MEM, READ_MEM, BROADCAST_INVALIDATE…).
     *
     * @param instruction_index Índice (0-based) de la instrucción a convertir.
     * @return Message completo con todos los campos decodificados.
     * @throws std::out_of_range Si instruction_index está fuera del rango válido.
     * @throws std::invalid_argument Si la instrucción no cumple el formato esperado.
     */
    Message convert_to_message(int instruction_index);

/* ----------------------------------- Getters & Setters --------------------------------------- */

    /** @brief Devuelve el identificador del PE. */
    int get_id() const;
    
    /** @brief Devuelve el valor de QoS asignado. */
    uint8_t get_qos() const;

    /** @brief Consulta el estado actual del PE. */
    PEState get_state() const;
    /** @brief Cambia el estado del PE. */
    void set_state(PEState s);

    /** @brief Obtiene el estado de respuesta actual del PE. */
    PEResponseState get_response_state() const;
    /** @brief Establece un nuevo estado de respuesta en el PE. */
    void set_response_state(PEResponseState state);

    /** @brief Devuelve el valor del Program Counter. */
    uint64_t get_pc() const;
    /** @brief Establece el valor del Program Counter. */
    void set_pc(uint64_t pc);

    /** @brief Devuelve la instrucción de 64 bits actual. */
    uint64_t get_actual_instruction() const;
    /** @brief Establece la instrucción de 64 bits actual. */
    void set_actual_instruction(uint64_t instr);

    /** @brief Devuelve una referencia mutable al mensaje actual. */
    Message& get_actual_message();
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

/* ------------------------------------------ Testing ------------------------------------------ */

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
    PEState             state_{PEState::IDLE};  /**< Estado del PE */
    PEResponseState     resp_state_{PEResponseState::READY};    /**< Estado del PE */
    uint64_t            pc_;                    /**< Program Counter. */
    uint64_t            actual_instruction_;    /**< Instrucción de 64 bits. */
    Message             actual_message_;        /**< Mensaje de prueba para debug. */
};
