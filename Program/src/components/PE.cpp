#include "../../include/components/PE.h"
#include <iostream>

PE::PE(int id, uint8_t qos)
    : id_(id), qos_(qos), pc_(0), instruction_memory_(id), actual_instruction_(0) {
    std::cout << "\n[PE] Created PE " << id_ << " with QoS=" << static_cast<int>(qos_) << "\n";
    // Inicializa el instruction memory de una vez
    instruction_memory_.initialize();
}

int PE::get_id() const {
    return id_;
}

uint8_t PE::get_qos() const {
    return qos_;
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
