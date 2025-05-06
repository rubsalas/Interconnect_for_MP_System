#include "../../include/components/PE.h"
#include <iostream>
#include <bitset>

PE::PE(int id, uint8_t qos)
    : instruction_memory_(id), id_(id), qos_(qos), pc_(0), actual_instruction_(0), 
    actual_message_(Operation::UNDEFINED, id, -1, 0, qos, 0, 0, 0, 0, 0, {}) {
    std::cout << "\n[PE] Created PE " << id_ << " with QoS=" << static_cast<int>(qos_) << "\n";
    // Inicializa el instruction memory de una vez
    instruction_memory_.initialize();
}

/* ----------------------------------- Getters & Setters --------------------------------------- */

int PE::get_id() const {
    return id_;
}

uint8_t PE::get_qos() const {
    return qos_;
}

PEState PE::get_state() const {
    return state_;
}

void PE::set_state(PEState s) {
    state_ = s;
}

uint64_t PE::get_pc() const {
    return pc_;
}

void PE::set_pc(uint64_t pc) {
    pc_ = pc;
}

uint64_t PE::get_actual_instruction() const {
    return actual_instruction_;
}

void PE::set_actual_instruction(uint64_t instr) {
    actual_instruction_ = instr;
}

void PE::set_actual_message(const Message& msg) {
    actual_message_ = msg;
}

const Message& PE::get_actual_message() const {
    return actual_message_;
}

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

void PE::debug_print() const {
    // 1) Cabecera de debug con ID, QoS y estado
    std::cout << "[PE] Debug: ID=" << id_
              << ", QoS=0x" << std::hex << static_cast<int>(qos_)
              << std::dec
              << ", State=" << state_to_string()
              << "\n";

    // 2) Mostrar el valor actual del Program Counter
    std::cout << "    PC=" << pc_ << "\n";

    // 3) Mostrar la instrucción actual
    std::cout << "    Actual Instruction=0x"
              << std::hex << actual_instruction_
              << " (bin: " << std::bitset<64>(actual_instruction_) << ")"
              << std::dec
              << "\n";

    // 4) Se podría listar otros datos de debug:
    //    - Número de mensajes pendientes
    //    - Estadísticas de stall, etc.
}

const char* PE::state_to_string() const {
    // Devolver el nombre asociado a state_
    switch (state_) {
        case PEState::IDLE:     return "IDLE";
        case PEState::RUNNING:  return "RUNNING";
        case PEState::STALLED:  return "STALLED";
        case PEState::FINISHED: return "FINISHED";
        default:                return "UNKNOWN";
    }
}

/* --------------------------------------------------------------------------------------------- */
