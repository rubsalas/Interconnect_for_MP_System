#pragma once

#include <iostream>
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
            std::vector<std::vector<uint8_t>>data = {});

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

    /**
     * @brief Devuelve el bloque de datos (payload) asociado al mensaje.
     * 
     * @return Referencia constante al vector de líneas de bytes.
     */
    const std::vector<std::vector<uint8_t>>& get_data() const;

    /** @brief Devuelve el ID de broadcast asociado o 0 si no aplica. */
    uint32_t get_broadcast_id() const;
    
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

    /**
     * @brief Establece el bloque de datos (payload) que acompaña al mensaje.
     * 
     * Normalmente se usa para llevar los bytes leídos o a escribir
     * en el cache/Memory.
     * 
     * @param data Bloque de datos como vector de líneas, donde cada línea
     *             es un vector de bytes.
     */
    void set_data(std::vector<std::vector<uint8_t>>& data);

    /** @brief Asigna un ID de broadcast para correlacionar INV_LINE/INV_ACK. */
    void set_broadcast_id(uint32_t id);
    
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

    /// @brief Suma a la latencia existente.
    void increment_latency(uint32_t cycles);

    /**
     * @brief Decrementa la latencia restante.
     * @param cycles Ciclos a restar (por defecto 1). Nunca baja de 0.
     */
    void decrement_latency(uint32_t cycles = 1);

    /********************/

    /// @brief Devuelve la latencia acumulada de este mensaje.
    uint32_t get_full_latency() const;

    /// @brief Fija la latencia total de este mensaje.
    void set_full_latency(uint32_t latency);

    /// @brief Suma @p delta ciclos a la latencia existente.
    void increment_full_latency(uint32_t delta);

    /// @brief Resta @p delta ciclos de la latencia (sin bajar de 0).
    void decrement_full_latency(uint32_t delta = 1);

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

    /**
     * @brief Convierte un valor de la enumeración Operation a su nombre en texto.
     *
     * Esta función permite obtener la representación textual de una operación
     * para facilitar debugging y logging.
     *
     * @param op Operación a convertir.
     * @return Cadena constante con el nombre de la operación.
     */
    static const char* OperationName(Operation op);

    /**
     * @brief Genera una representación en texto del mensaje para depuración.
     * @return Cadena con todos los campos del mensaje.
     */
    std::string to_string() const;

    /** 
     * @brief Imprime por consola información de debug de la latencia restante. 
     */
    void print_latency_debug() const;

    /** 
     * @brief Imprime por consola información de debug de la latencia total. 
     */
    void print_full_latency_debug() const;

/* --------------------------------------------------------------------------------------------- */

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
    std::vector<std::vector<uint8_t>> data_;    /**< Payload de datos. */

    uint32_t latency_{0};           /**< Latencia restante en ciclos. */
    uint32_t full_latency_{0};      /**< Latencia total de la instruccion */

    uint32_t broadcast_id_{0};      /**< ID del Broadcast, si es un Message de esos. */
};

