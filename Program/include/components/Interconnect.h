#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include "Message.h"

/**
 * @enum ArbitScheme
 * @brief Esquemas de arbitraje soportados por el Interconnect.
 */
enum class ArbitScheme {
    FIFO,        /**< Primer en entrar, primer en salir */
    PRIORITY     /**< Basado en valores de QoS */
};

/**
 * @class Interconnect
 * @brief Modela el bus/fabric que enruta mensajes entre PEs y memoria.
 *
 * Gestiona colas de petición, aplica arbitraje y entrega respuestas.
 */
class Interconnect {
public:
    /**
     * @brief Construye un Interconnect para un número de PEs y un esquema de arbitraje.
     * @param num_pes Cantidad de PEs conectados.
     * @param scheme Esquema de arbitraje a utilizar.
     */
    Interconnect(int num_pes, ArbitScheme scheme);

    /**
     * @brief Encola una petición entrante de forma thread-safe.
     * @param m Mensaje que representa la petición.
     */
    void push_message(const Message& m);

    /**
     * @brief Avanza un "tick" de simulación: procesa colas según el arbitraje.
     */
    void tick();

    /**
     * @brief Imprime en consola configuración y esquema actuales.
     */
    void debug_print() const;

    /**
     * @brief Imprime en consola todas las peticiones pendientes en la cola interna.
     *
     * Hace una copia temporal de la cola para no vaciarla, y muestra cada
     * Message::to_string(). Usa un mutex para acceso thread-safe.
     */
    void debug_print_request_queue();



private:
    int num_pes_;                                  /**< Número de PEs conectados */
    ArbitScheme scheme_;                           /**< Esquema de arbitraje */
    std::queue<Message> in_queue_;      /**< Cola de Messages entrantes */
    std::mutex          in_queue_mtx_;  /**< Protege in_queue_ contra accesos concurrentes. */
    std::queue<Message> out_queue_;     /**< Cola de Messages salientes */
};
