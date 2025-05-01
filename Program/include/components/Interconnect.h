#pragma once

#include <vector>
#include <queue>
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
     * @brief Recibe una petición (mensaje) de un PE y la encola.
     * @param m Mensaje enviado por un PE.
     */
    void send_request(const Message& m);

    /**
     * @brief Avanza un "tick" de simulación: procesa colas según el arbitraje.
     */
    void tick();

    /**
     * @brief Consulta si hay respuestas disponibles para un PE dado.
     * @param pe_id Identificador del PE.
     * @return true si hay al menos un mensaje de respuesta.
     */
    bool has_response(int pe_id) const;

    /**
     * @brief Obtiene la siguiente respuesta para un PE.
     * @param pe_id Identificador del PE.
     * @return Mensaje de respuesta.
     */
    Message get_response(int pe_id);

    /**
     * @brief Muestra las estadísticas del Interconnect.
     */
    void report_stats() const;

    /**
     * @brief Imprime en consola configuración y esquema actuales.
     */
    void debug_print() const;

private:
    int num_pes_;                                  /**< Número de PEs conectados */
    ArbitScheme scheme_;                           /**< Esquema de arbitraje */
    std::vector<std::queue<Message>> in_queues_;   /**< Colas de peticiones por PE */
    std::vector<std::queue<Message>> out_queues_;  /**< Colas de respuestas por PE */
};
