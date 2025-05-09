#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @enum Operation
 * @brief Operaciones soportadas por el Interconnect según la especificación.
 */
enum class Operation {
    READ_MEM,               /**< Solicitud de lectura de memoria principal */
    WRITE_MEM,              /**< Solicitud de escritura en memoria principal */
    BROADCAST_INVALIDATE,   /**< Invalidación de línea de caché en todos los PEs */
    INV_LINE,               /**< Interconnect comanda a PEs a invalidar linea */
    INV_ACK,                /**< Acknowledgment de invalidación de un PE */
    INV_COMPLETE,           /**< Confirmación de finalización de todas las invalidaciones */
    READ_RESP,              /**< Respuesta con datos de lectura */
    WRITE_RESP,             /**< Respuesta con estado de escritura */
    END,                    /**< Fin de ejecucion de instrucciones por PE */
    UNDEFINED               /**< Operación no definida */
};

/**
 * @class Message
 * @brief Representa un paquete de comunicación entre un PE y el Interconnect.
 *
 * Contiene todos los campos mínimos definidos en la tabla de mensajes,
 * permitiendo modelar solicitudes de lectura/escritura, invalidaciones y sus respuestas.
 */
class Message {
public:
    /**
     * @brief Construye un mensaje completo con todos los atributos posibles.
     * @param operation Tipo de operación (READ_MEM, WRITE_MEM, etc.).
     * @param src        Identificador del PE origen (-1 si no aplica).
     * @param dst        Identificador del PE destino (-1 para broadcast).
     * @param addr       Dirección de memoria asociada al mensaje.
     * @param qos        Calidad de servicio (0x00-0xFF).
     * @param size       Número de bytes a leer o escribir.
     * @param num_lines  Número de líneas de caché involucradas.
     * @param start_line Índice de la línea de caché inicial.
     * @param cache_line Línea de caché específica (para invalidación).
     * @param status     Código de estado de escritura (0x1 OK, 0x0 NOT_OK).
     * @param data       Vector de datos (palabras de 32 bits) para transferencias.
     */
    Message(Operation operation,
            int src = -1,               // SRC
            int dst = -1,               // DEST
            uint64_t addr = 0,          // ADDR
            uint8_t qos = 0,            // QoS
            uint32_t size = 0,          // SIZE
            uint32_t num_lines = 0,     // NUM_OF_CACHE_LINES
            uint32_t start_line = 0,    // START_CACHE_LINE
            uint32_t cache_line = 0,    // CACHE_LINE
            uint32_t status = 0,        // STATUS
            std::vector<uint32_t> data = {});

/* ----------------------------------- Getters & Setters --------------------------------------- */

    // Getters
    Operation get_operation() const;
    int get_src_id() const;
    int get_dest_id() const;
    uint64_t get_address() const;
    uint8_t get_qos() const;
    uint32_t get_size() const;
    uint32_t get_num_lines() const;
    uint32_t get_start_line() const;
    uint32_t get_cache_line() const;
    uint32_t get_status() const;
    const std::vector<uint32_t>& get_data() const;

    // Setters
    void set_operation(Operation op);
    void set_src_id(int id);
    void set_dest_id(int id);
    void set_address(uint64_t addr);
    void set_qos(uint8_t q);
    void set_size(uint32_t s);
    void set_num_lines(uint32_t n);
    void set_start_line(uint32_t sl);
    void set_cache_line(uint32_t cl);
    void set_status(uint32_t st);
    void set_data(const std::vector<uint32_t>& d);

/* --------------------------------------------------------------------------------------------- */

/* -------------------------------------- Latency Handler -------------------------------------- */

    /**
     * @brief Establece la latencia (en ciclos) que debe “recorrer” este mensaje.
     * @param cycles Número de ciclos de latencia.
     */
    void set_latency(uint32_t cycles);

    /**
     * @brief Obtiene la latencia restante en ciclos.
     * @return Ciclos que faltan para completar la transmisión.
     */
    uint32_t get_latency() const;

    /**
     * @brief Decrementa la latencia restante.
     * @param cycles Ciclos a restar (por defecto 1). Nunca baja de 0.
     */
    void decrement_latency(uint32_t cycles = 1);

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */


    /**
     * @brief Genera una representación en texto del mensaje para depuración.
     * @return Cadena con todos los campos del mensaje.
     */
    std::string to_string() const;

private:
    Operation operation_{Operation::UNDEFINED}; /**< Tipo de operación. */
    int src_id_{-1};                /**< ID del PE origen. */
    int dest_id_{-1};               /**< ID del PE destino (-1=Broadcast). */
    uint64_t address_{0};           /**< Dirección de memoria. */
    uint8_t qos_{0};                /**< Calidad de servicio. */
    uint32_t size_{0};              /**< Tamaño en bytes. */
    uint32_t num_lines_{0};         /**< Número de líneas de caché. */
    uint32_t start_line_{0};        /**< Primera línea de caché. */
    uint32_t cache_line_{0};        /**< Línea de caché específica. */
    uint32_t status_{0};            /**< Código de estado de la operación. */
    std::vector<uint32_t> data_;    /**< Payload de datos. */

    uint32_t latency_{0};           /**< Latencia restante en ciclos. */
};

/* --------------------------------------------------------------------------------------------- */
