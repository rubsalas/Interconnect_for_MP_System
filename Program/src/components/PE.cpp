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

void PE::pc_plus_4() {
    // Sumamos 1 al contador de programa
    ++pc_;
}

Message PE::convert_to_message(int instruction_index) {
    std::string bin = instruction_memory_.get_instruction(instruction_index);
    uint64_t instr = std::bitset<64>(bin).to_ullong();
    
    Message msg(Operation::UNDEFINED);

    uint8_t opcode = (instr >> 41) & 0b11;
    msg.set_src_id((instr >> 36) & 0b11111);

    switch (opcode) {
        case 0b00: // WRITE_MEM
            msg.set_operation(Operation::WRITE_MEM);
            msg.set_address((instr >> 20) & 0xFFFF);
            msg.set_num_lines((instr >> 12) & 0xFF);
            msg.set_start_line((instr >> 4) & 0xFF);
            msg.set_qos(qos_);
            break;
            

        case 0b01: // READ_MEM
            msg.set_operation(Operation::READ_MEM);
            msg.set_address((instr >> 20) & 0xFFFF);
            msg.set_size((instr >> 12) & 0xFF);
            msg.set_qos(qos_);
            break;
            

        case 0b10: // BROADCAST_INVALIDATE
            msg.set_operation(Operation::BROADCAST_INVALIDATE);
            msg.set_cache_line((instr >> 20) & 0xFF);
            msg.set_qos(qos_); 
            break;

        default:
            msg.set_operation(Operation::UNDEFINED);  
            break; 

    return msg;
    }
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

PEResponseState PE::get_response_state() const {
    return resp_state_;
}

void PE::set_response_state(PEResponseState state) {
    resp_state_ = state;
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

// non-const overload
Message& PE::get_actual_message() {
    return actual_message_;
}

const Message& PE::get_actual_message() const {
    return actual_message_;
}

void PE::set_actual_message(const Message& msg) {
    actual_message_ = msg;
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
