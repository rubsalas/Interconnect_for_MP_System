#include "../../include/components/PE.h"
#include <iostream>
#include "../../include/Message.h"
#include "../../include/components/Instruction_Memory.h"
#include <bitset>



PE::PE(int id, uint8_t qos)
    : id_(id), qos_(qos), pc_(0), instruction_memory_(id), actual_instruction_(0) {
    std::cout << "\n[PE] Created PE " << id_ << " with QoS=" << static_cast<int>(qos_) << "\n";
    // Inicializa el instruction memory de una vez
    instruction_memory_.initialize();
}

Message convert_to_message(const InstructionMemory& instruction_memory_, int index) {
    std::string bin = instruction_memory_.get_instruction(index);
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
            msg.set_qos(instr & 0xF);
            

        case 0b01: // READ_MEM
            msg.set_operation(Operation::READ_MEM);
            msg.set_address((instr >> 20) & 0xFFFF);
            msg.set_size((instr >> 12) & 0xFF);
            msg.set_qos(instr & 0xF);
            

        case 0b10: // BROADCAST_INVALIDATE
            msg.set_operation(Operation::BROADCAST_INVALIDATE);
            msg.set_cache_line((instr >> 20) & 0xFF);
            msg.set_qos(instr & 0xF); 

        default:
            msg.set_operation(Operation::UNDEFINED);   

    return msg;
    }
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
