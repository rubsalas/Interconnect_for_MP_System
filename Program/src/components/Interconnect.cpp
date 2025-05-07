#include "../../include/components/Interconnect.h"
#include <algorithm>
#include <iostream>

Interconnect::Interconnect(int num_pes, ArbitScheme scheme)
    : num_pes_(num_pes), scheme_(scheme) {
    std::cout << "[Interconnect] Created with " << num_pes_
              << " PE connections and will use " << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << " as its arbitration scheme.\n";
}

void Interconnect::push_message(const Message& m) {
    std::lock_guard<std::mutex> lock(in_queue_mtx_);

    if (scheme_ == ArbitScheme::FIFO) {
        // FIFO puro: al final
        in_queue_.push_back(m);

    } else { // PRIORITY basada en QoS
        // busco la primera posición donde QoS de existing < QoS de m
        auto it = std::find_if(
            in_queue_.begin(), in_queue_.end(),
            [&](auto const& existing){
                return existing.get_qos() < m.get_qos();
            }
        );
        // inserto allí (si it==end(), inserta al final)
        in_queue_.insert(it, m);
    }

    std::cout << "[Interconnect] Safely queued message: "
              << m.to_string() << "\n";
}

Message Interconnect::pop_next() {
    // 1) Bloqueamos el mutex
    std::lock_guard<std::mutex> lock(in_queue_mtx_);
    if (in_queue_.empty()) {
        throw std::out_of_range("Interconnect::pop_next(): queue is empty");
    }
    // 2) Copiamos el mensaje que está al frente
    Message m = in_queue_.front();
    // 3) Lo eliminamos de la deque
    in_queue_.pop_front();
    // 4) Devolvemos esa copia
    return m;
}

void Interconnect::tick() {
    std::cout << "[Interconnect] tick: processing messages according to scheme "
              << static_cast<int>(scheme_) << "\n";
    // TODO: apply arbitration and move messages between queues
}

/* ---------------------------------------- Testing -------------------------------------------- */

void Interconnect::debug_print() const {
    std::cout << "\n[Interconnect] Debug: num_pes=" << num_pes_
              << ", scheme=" << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << "\n";
    // TODO: listar queues o estadísticas básicas aquí.
}

void Interconnect::debug_print_request_queue() const {
    std::lock_guard<std::mutex> lock(in_queue_mtx_);
    std::cout << "[Interconnect] Pending requests in in_queue_:\n";

    std::deque<Message> copy = in_queue_;
    if (copy.empty()) {
        std::cout << "  (none)\n";
        return;
    }

    size_t idx = 0;
    while (!copy.empty()) {
        std::cout << "  [" << idx++ << "] "
                  << copy.front().to_string() << "\n";
        copy.pop_front();
    }
}

/* --------------------------------------------------------------------------------------------- */
