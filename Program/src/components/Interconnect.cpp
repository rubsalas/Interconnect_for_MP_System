#include "../../include/components/Interconnect.h"
#include <iostream>

Interconnect::Interconnect(int num_pes, ArbitScheme scheme)
    : num_pes_(num_pes), scheme_(scheme) {
    in_queues_.resize(num_pes_);
    out_queues_.resize(num_pes_);
    std::cout << "[Interconnect] Created with " << num_pes_
              << " PEs, scheme=" << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << "\n";
}

void Interconnect::send_request(const Message& m) {
    std::cout << "[Interconnect] send_request: src=" << m.src_id()
              << " op=" << static_cast<int>(m.operation()) << "\n";
    // TODO: enqueue into in_queues_[m.src_id()]
}

void Interconnect::tick() {
    std::cout << "[Interconnect] tick: processing messages according to scheme "
              << static_cast<int>(scheme_) << "\n";
    // TODO: apply arbitration and move messages between queues
}

bool Interconnect::has_response(int pe_id) const {
    std::cout << "[Interconnect] has_response check for PE " << pe_id << "\n";
    // TODO: return !out_queues_[pe_id].empty();
    return false;
}

Message Interconnect::get_response(int pe_id) {
    std::cout << "[Interconnect] get_response for PE " << pe_id << "\n";
    // TODO: pop from out_queues_[pe_id]
    return Message(Operation::UNDEFINED);
}


void Interconnect::report_stats() const {
    std::cout << "[Interconnect] report_stats: (placeholder)\n";
    // TODO: print statistics collected
}

void Interconnect::debug_print() const {
    std::cout << "[Interconnect] Debug: num_pes=" << num_pes_
              << ", scheme=" << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << "\n";
    // TODO: listar queues o estadísticas básicas aquí.
}
