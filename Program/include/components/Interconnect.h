#pragma once

#include <vector>
#include <deque>
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

enum class ICState {
    IDLE,           /**< No hay peticiones pendientes. */
    //RECEIVING,      /**< Leyendo nuevas solicitudes en in_queue_. */
    PROCESSING,     /**< Decodificando la petición y preparando la acción. */
    //MEM_ACCESS,     /**< Modelando la latencia de acceso a memoria principal. */
    //RESPONDING,     /**< Encolando y despachando las respuestas a los PEs. */
    FINISHED        /**< No entrarán mas mensajes y se han respondido todos */
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
     * @brief Avanza un "tick" de simulación: procesa colas según el arbitraje.
     */
    void tick();

/* ------------------------------------- Message Handling -------------------------------------- */

    /**
     * @brief Comprueba si todas las colas internas están vacías.
     *
     * Esto abarca:
     *  - Cola de peticiones entrantes (in_queue_).
     *  - Cola de mensajes en ejecución (mid_processing_queue_).
     *  - (Opcional) otras colas como la de respuestas si las implementas.
     *
     * @return true si ninguna cola contiene mensajes pendientes.
     */
    bool all_queues_empty() const;

/* ------------------------------------ */
/*                                      */
/*               in_queue               */
/*                                      */
/* ------------------------------------ */

    /**
     * @brief Encola una petición entrante de forma thread-safe a in_queue.
     * @param m Mensaje que representa la petición.
     */
    void push_message(const Message& m);

    /**
     * @brief Extrae el siguiente mensaje según el esquema de in_queue.
     * @return Mensaje a procesar.
     * @throws std::out_of_range si la cola está vacía.
     */
    Message pop_next();

    /** @brief Devuelve true si la cola de entrada está vacía. */
    bool in_queue_empty() const;

/* ------------------------------------ */
/*                                      */
/*         mid_processing_queue         */
/*                                      */
/* ------------------------------------ */

    /**
     * @brief Encola un mensaje en mid_processing_queue_.
     *
     * Esta cola simula la latencia de los mensajes “en vuelo” dentro
     * del pipeline del Interconnect.
     *
     * @param m Mensaje a encolar en la etapa media.
     */
    void push_mid_processing(const Message& m);

    /**
     * @brief Extrae y elimina el mensaje en la posición dada de la cola intermedia.
     *
     * Este método se usa cuando ya sabes qué índice (i) ha completado su latencia
     * y quieres retirarlo del pipeline.
     *
     * @param index Índice (0‐based) del mensaje a extraer.
     * @return Copia del Message que estaba en esa posición.
     * @throws std::out_of_range si index >= tamaño de la cola.
     */
    Message pop_mid_processing_at(size_t index);

    /** @brief Devuelve true si la cola intermedia de procesamiento está vacía. */
    bool mid_processing_empty() const;

    /** @brief Devuelve el número de mensajes actualmente en mid_processing_queue_. */
    size_t mid_processing_size() const;

/* ------------------------------------ */
/*                                      */
/*               out_queue              */
/*                                      */
/* ------------------------------------ */


    /**
     * @brief Encola un mensaje en la cola de respuestas salientes según el esquema de arbitraje.
     *
     * Inserta el mensaje en out_queue_ de la siguiente manera:
     * - Si scheme_ == FIFO, lo pone al final de la cola.
     * - Si scheme_ == PRIORITY, lo inserta en orden descendente de QoS.
     *
     * Esta cola contiene las respuestas que ya han completado su latencia
     * en mid_processing_queue_ y están listas para despacharse a los PEs.
     *
     * @param m Mensaje de respuesta a encolar en out_queue_.
     */
    void push_out_queue(const Message& m);


    /**
     * @brief Extrae y elimina el mensaje en la posición dada de out_queue_.
     *
     * Útil cuando sabes qué índice de respuesta ya debe ser despachado
     * y quieres retirarlo de la cola de salida.
     *
     * @param index Índice (0‐based) del mensaje a extraer.
     * @return Copia del Message que estaba en esa posición.
     * @throws std::out_of_range si index >= tamaño de out_queue_.
     */
    Message pop_out_queue_at(size_t index);

    /**
     * @brief Devuelve true si la cola de respuestas salientes está vacía.
     * @return true si no hay mensajes pendientes en out_queue_.
     */
    bool out_queue_empty() const;

/* --------------------------------------------------------------------------------------------- */


/* ----------------------------------- Getters & Setters --------------------------------------- */

    /** @brief Lee el estado actual del Interconnect. */
    ICState get_state() const;

    /** @brief Fuerza un nuevo estado (interno). */
    void set_state(ICState s);

    /**
     * @brief Obtiene la cola de mensajes entrantes.
     * @return Referencia constante a la deque interna de mensajes entrantes.
     */
    const std::deque<Message>& get_in_queue() const;

    /**
     * @brief Reemplaza la cola de mensajes entrantes.
     * @param q Nueva deque de mensajes a asignar.
     *
     * Esta operación se realiza de forma thread-safe, bloqueando el mutex
     * que protege la cola antes de la asignación.
     */
    void set_in_queue(const std::deque<Message>& q);

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

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
    void debug_print_in_queue() const;

    /**
     * @brief Imprime en consola todas las peticiones que están en la cola intermedia.
     *
     * Hace una copia de mid_processing_queue_ para no alterar el estado real,
     * y lista cada Message usando Message::to_string().
     */
    void debug_print_mid_processing_queue() const;

    /**
     * @brief Imprime en consola todas las respuestas pendientes en out_queue_.
     *
     * Hace una copia de out_queue_ para no alterar la cola real,
     * y lista cada Message usando Message::to_string().
     */
    void debug_print_out_queue() const;

/* --------------------------------------------------------------------------------------------- */

private:
    int                 num_pes_;               /**< Número de PEs conectados */
    ArbitScheme         scheme_;                /**< Esquema de arbitraje */
    ICState             state_{ICState::IDLE};  /**< Estado del Interconnect */
    
    std::deque<Message> in_queue_;              /**< Cola de Messages entrantes */
    mutable std::mutex  in_queue_mtx_;          /**< Protege in_queue_ contra accesos concurrentes. */
    
    std::deque<Message> mid_processing_queue_;  /**< Cola de Messages en ejecucion */
    mutable std::mutex  mid_processing_mtx_;    /**< Protege mid_processing_queue_ */

    std::deque<Message> out_queue_;             /**< Cola de respuestas salientes */
    mutable std::mutex  out_queue_mtx_;         /**< Protege out_queue_ */
};
