#include "../../include/components/Interconnect.h"
#include <algorithm>
#include <iostream>

Interconnect::Interconnect(int num_pes, ArbitScheme scheme)
    : num_pes_(num_pes), scheme_(scheme) {
    std::cout << "[Interconnect] Created with " << num_pes_
              << " PE connections and will use " << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << " as its arbitration scheme.\n";
}

uint32_t Interconnect::register_broadcast(int origin_pe) {
    // 1) Conseguir un ID único
    uint32_t bid = next_broadcast_id_.fetch_add(1, std::memory_order_relaxed);

    {
        // 2) Guardamos la info bajo mutex
        std::lock_guard<std::mutex> lk(broadcast_mtx_);
        pending_broadcasts_[bid] = PendingBroadcast{
            .origin_pe    = origin_pe,
            .pending_acks = num_pes_   // esperamos uno por cada PE
        };
    }

    // 3) Devolvemos el ID para que System pueda asignarlo al Message
    return bid;
}

/* ------------------------------------- Message Handling -------------------------------------- */

bool Interconnect::all_queues_empty() const {
    // 1) Bloquear ambas colas para chequeo consistente
    std::lock_guard<std::mutex> lock_in(in_queue_mtx_);
    std::lock_guard<std::mutex> lock_mid(mid_processing_mtx_);
    std::lock_guard<std::mutex> lock_out(out_queue_mtx_);

    // 2) Comprobar que todas estén vacías
    return in_queue_.empty() && mid_processing_queue_.empty() && out_queue_.empty();
}

/* ------------------------------------ */
/*                                      */
/*               in_queue               */
/*                                      */
/* ------------------------------------ */

void Interconnect::push_message(const Message& m) {
    std::lock_guard<std::mutex> lock(in_queue_mtx_);

    /* Ordenamiento segun esquema de arbitraje */
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

    /*std::cout << "[Interconnect] Safely queued new message: "
              << m.to_string() << "\n";*/
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

bool Interconnect::in_queue_empty() const {
    std::lock_guard<std::mutex> lock(in_queue_mtx_);
    return in_queue_.empty();
}

/* ------------------------------------ */
/*                                      */
/*         mid_processing_queue         */
/*                                      */
/* ------------------------------------ */

void Interconnect::push_mid_processing(const Message& m) {
    // 1) Bloqueo para acceso concurrente
    std::lock_guard<std::mutex> lock(mid_processing_mtx_);
    // 2) Encolamos al final de la cola intermedia
    mid_processing_queue_.push_back(m);
    // 3) Debug opcional
    /*std::cout << "[Interconnect] Enqueued to mid_processing: "
              << m.to_string() << "\n";*/
}

Message Interconnect::pop_mid_processing_at(size_t index) {
    std::lock_guard<std::mutex> lock(mid_processing_mtx_);
    if (index >= mid_processing_queue_.size()) {
        throw std::out_of_range("pop_mid_processing_at: index out of range");
    }
    // Iterator to the element at `index`
    auto it = mid_processing_queue_.begin() + index;
    Message m = *it;
    // Erase that element from the deque
    mid_processing_queue_.erase(it);
    return m;
}

bool Interconnect::mid_processing_empty() const {
    std::lock_guard<std::mutex> lock(mid_processing_mtx_);
    return mid_processing_queue_.empty();
}

size_t Interconnect::mid_processing_size() const {
    std::lock_guard<std::mutex> lock(mid_processing_mtx_);
    return mid_processing_queue_.size();
}

/* ------------------------------------ */
/*                                      */
/*               out_queue              */
/*                                      */
/* ------------------------------------ */


void Interconnect::push_out_queue(const Message& m) {
    // 1) Bloqueamos el mutex para asegurar acceso exclusivo
    std::lock_guard<std::mutex> lock(out_queue_mtx_);

    // 2) Encolado según el esquema de arbitraje
    if (scheme_ == ArbitScheme::FIFO) {
        // FIFO puro: al final de la cola
        out_queue_.push_back(m);
    } else { // PRIORITY basada en QoS
        // Insertar antes del primer mensaje con QoS menor
        auto it = std::find_if(
            out_queue_.begin(), out_queue_.end(),
            [&](auto const& existing) {
                return existing.get_qos() < m.get_qos();
            }
        );
        out_queue_.insert(it, m);
    }

    // 3) Debug: mostramos que encolamos la respuesta
    /*std::cout << "[Interconnect] Safely queued to out_queue: "
              << m.to_string() << "\n";*/
}

Message Interconnect::pop_out_queue_at(size_t index) {
    std::lock_guard<std::mutex> lock(out_queue_mtx_);
    if (index >= out_queue_.size()) {
        throw std::out_of_range("pop_out_queue_at: index out of range");
    }
    auto it = out_queue_.begin() + index;
    Message m = *it;
    out_queue_.erase(it);
    return m;
}

bool Interconnect::out_queue_empty() const {
    std::lock_guard<std::mutex> lock(out_queue_mtx_);
    return out_queue_.empty();
}

/* --------------------------------------------------------------------------------------------- */

/* ----------------------------------- Getters & Setters --------------------------------------- */

ICState Interconnect::get_state() const {
    return state_;
}

void Interconnect::set_state(ICState s) {
    state_ = s;
}

const std::deque<Message>& Interconnect::get_in_queue() const {
    // No bloqueamos aquí, porque devolvemos solo lectura.
    return in_queue_;
}

void Interconnect::set_in_queue(const std::deque<Message>& q) {
    std::lock_guard<std::mutex> lock(in_queue_mtx_);
    in_queue_ = q;
}

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

void Interconnect::debug_print() const {
    std::cout << "\n[Interconnect] Debug: num_pes=" << num_pes_
              << ", scheme=" << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY")
              << "\n";
    // TODO: listar queues o estadísticas básicas aquí.
}

void Interconnect::debug_print_in_queue() const {
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

    // Línea en blanco al final para claridad
    std::cout << std::endl;
}

void Interconnect::debug_print_mid_processing_queue() const {
    // 1) Bloqueamos el mutex para proteger la cola
    std::lock_guard<std::mutex> lock(mid_processing_mtx_);

    // 2) Cabecera para identificar la salida
    std::cout << "[Interconnect] Pending messages in mid_processing_queue_:\n";

    // 3) Hacemos una copia para iterar sin modificar la cola real
    std::deque<Message> copy = mid_processing_queue_;
    if (copy.empty()) {
        std::cout << "  (none)\n";
        return;
    }

    // 4) Iteramos y mostramos cada mensaje
    size_t idx = 0;
    while (!copy.empty()) {
        const Message& m = copy.front();
        std::cout << "  [" << idx++ << "] " << m.to_string() << "\n";
        copy.pop_front();
    }

    // 5) Línea en blanco al final para claridad
    std::cout << std::endl;
}

void Interconnect::debug_print_out_queue() const {
    // 1) Bloqueamos el mutex para proteger la cola
    std::lock_guard<std::mutex> lock(out_queue_mtx_);

    // 2) Cabecera para identificar la salida
    std::cout << "[Interconnect] Pending messages in out_queue_:\n";

    // 3) Hacemos una copia para iterar sin modificar la cola real
    std::deque<Message> copy = out_queue_;
    if (copy.empty()) {
        std::cout << "  (none)\n";
        return;
    }

    // 4) Iteramos y mostramos cada mensaje
    size_t idx = 0;
    while (!copy.empty()) {
        const Message& m = copy.front();
        std::cout << "  [" << idx++ << "] " << m.to_string() << "\n";
        copy.pop_front();
    }

    // 5) Línea en blanco al final para claridad
    std::cout << std::endl;
}

/* --------------------------------------------------------------------------------------------- */
