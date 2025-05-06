#include "../../include/components/Interconnect.h"
#include <iostream>

Interconnect::Interconnect(int num_pes, ArbitScheme scheme)
    : num_pes_(num_pes), scheme_(scheme) {
    std::cout << "[Interconnect] Created with " << num_pes_
              << " PE connections and will use " << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << " as its arbitration scheme.\n";
}

void Interconnect::push_message(const Message& m) {
    // 1) Bloqueamos el mutex para asegurar que
    //    solo un hilo a la vez modifica in_queue_.
    std::lock_guard<std::mutex> lock(in_queue_mtx_);

    // 2) Encolamos el mensaje en in_queue_
    in_queue_.push(m);

    // 3) Debug: informamos en consola
    std::cout << "[Interconnect] Safely queued message: "
              << m.to_string() << "\n";

    // TODO: Acomodar el queue dependiendo del QoS si el scheme esta en PRIORITY

    // Al llegar al fin de la funcion, el destructor de lock_guard
    // libera automaticamente el mutex, incluso si ocurre una excepción.
}

void Interconnect::tick() {
    std::cout << "[Interconnect] tick: processing messages according to scheme "
              << static_cast<int>(scheme_) << "\n";
    // TODO: apply arbitration and move messages between queues
}

void Interconnect::debug_print() const {
    std::cout << "\n[Interconnect] Debug: num_pes=" << num_pes_
              << ", scheme=" << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << "\n";
    // TODO: listar queues o estadísticas básicas aquí.
}

void Interconnect::debug_print_request_queue() {
    // Aseguramos acceso exclusivo
    std::lock_guard<std::mutex> lock(in_queue_mtx_);

    std::cout << "[Interconnect] Pending requests in in_queue_:\n";
    // Hacemos una copia para no alterar la cola real
    std::queue<Message> copy = in_queue_;
    if (copy.empty()) {
        std::cout << "  (none)\n";
        return;
    }

    size_t idx = 0;
    while (!copy.empty()) {
        const Message& m = copy.front();
        std::cout << "  [" << idx++ << "] " << m.to_string() << "\n";
        copy.pop();
    }
}
