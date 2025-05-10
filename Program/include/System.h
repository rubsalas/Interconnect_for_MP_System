#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "components/PE.h"
#include "components/Interconnect.h"
#include "components/Local_Cache.h"
#include "components/Shared_Memory.h"

/**
 * @class System
 * @brief Controlador principal de la simulación MP: gestiona PEs, Interconnect,
 *        ciclos y estadísticas.
 */
class System {
public:
    /**
     * @brief Construye un sistema con un número dado de PEs y esquema de arbitraje.
     * @param num_pes Cantidad de processing elements a instanciar.
     * @param scheme  Esquema de arbitraje para el Interconnect (FIFO o PRIORITY).
     */
    System(int total_pes, ArbitScheme scheme, bool stepping_enabled = true);

    /** @brief Inicializa PEs, caches, interconnect y memoria compartida. */
    void initialize();

/* ----------------------------------------- Execution ----------------------------------------- */

    /**  
     * @brief Habilita o deshabilita el modo stepping.
     * @param enable true = cada ciclo espera al usuario, false = corre automáticamente.
     */
    void set_stepping_enabled(bool enable);

    /** @brief Arranca todos los hilos de PEs y del Interconnect. */
    void run();

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Statistics ----------------------------------------- */

    /** @brief Reporta estadísticas finales tras la simulación. */
    void report_statistics() const;

    /**
     * @brief Convierte un Operation a cadena.
     * @param op Operación a convertir.
     * @return C-string con el nombre de la operación.
     */
    static const char* operation_to_string(Operation op);

    /**
     * @brief Registra en un archivo de texto las métricas de un mensaje.
     *
     * Cada llamada añade una línea al final de @p filename con el formato:
     *   [PE_id] [QoS] [Operation] [size_bytes] [num_bytes_lines] [full_latency] [bandwidth]
     *
     * - size_bytes       = msg.get_size() * 4  
     * - num_bytes_lines  = msg.get_num_lines() * 16  
     * - full_latency     = msg.get_full_latency()  
     * - bandwidth        = msg.get_bandwidth()
     *
     * @param msg       Mensaje cuyas métricas se van a loguear.
     * @param filename  Ruta al archivo de log (se abre en modo append).  
     *                  Por defecto "latency_log.txt".
     * @throws std::runtime_error si no se puede abrir el archivo.
     */
    void log_message_metrics(const Message& msg,
                             const std::string& filename = "latency_log.txt") const;

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

    /** @brief Imprime en consola el ID y QoS de cada PE para depuración. */
    void debug_print() const;

    void system_test_G(const std::string& file_path);
    void system_test_R();

/* --------------------------------------------------------------------------------------------- */

private:
    int                             total_pes_;             /**< Número de PEs configurados. */
    std::vector<PE>                 pes_;                   /**< Vector de PEs del sistema. */
    ArbitScheme                     scheme_;                /**< Esquema de arbitraje seleccionado. */
    std::unique_ptr<Interconnect>   interconnect_;          /**< Interconnect para enrutar mensajes. */
    std::vector<LocalCache>         caches_;                /**< Caches Locales L1 para cada PE. */
    std::unique_ptr<SharedMemory>   shared_memory_;         /**< Interconnect para enrutar mensajes. */

    // --------------------------------------------------
    // Stepping control
    // --------------------------------------------------
    bool stepping_enabled_;     /**< true = stepping, false = auto-run */
    std::mutex                      step_mtx_;              /**< Protege current_step_. */
    std::condition_variable         step_cv_;               /**< Despierta hilos en cada step. */
    int                             current_step_{0};       /**< Contador de pasos completados. */

    std::vector<std::thread>        pe_threads_;            /**< Hilos que ejecutan cada PE. */
    std::thread                     interconnect_thread_;   /**< Hilo para el Interconnect. */

    /** @brief Crea e instancia el Interconnect como unico */
    void initialize_interconnect();
    /** @brief Crea e inicializa los PEs con QoS por defecto. */
    void initialize_pes();
    /** @brief Inicializa un caché local por cada PE (sin QoS). */
    void initialize_caches();
    /** @brief Inicializa la memoria compartida del sistema. */
    void initialize_shared_memory();

    /**
     * @brief Incrementa current_step_ y despierta a todos los hilos bloqueados.
     */
    void step();

/* ------------------------------------ */
/*                                      */
/*             PE's threads             */
/*                                      */
/* ------------------------------------ */

    /**
     * @brief Lanza un hilo para el PE en la posición `pe_id` del vector pes_.
     *
     * Cada hilo ejecutará `pe_worker(pe_id)` y quedará registrado en pe_threads_.
     *
     * @param pe_id Índice del PE en pes_ (de 0 a total_pes_-1).
     */
    void start_pe_thread(int pe_id);

    /**
     * @brief Rutina que corre dentro de cada hilo de PE.
     *
     * Aquí es donde cada PE hará su ciclo de fetch/execute,
     * enviará mensajes al interconect y recibirá respuestas.
     *
     * @param pe_id Índice del PE en el vector pes_.
     */
    void pe_execution_cycle(int pe_id);

    /** @brief Espera a que todos los hilos de PE terminen. */
    void join_pe_threads();

/* ------------------------------------ */
/*                                      */
/*         Interconnect's thread        */
/*                                      */
/* ------------------------------------ */

    /** @brief Arranca el hilo que corre interconnect_execution_cycle(). */
    void start_interconnect_thread();
    /** @brief Rutina que corre en el hilo del Interconnect. */
    void interconnect_execution_cycle();
    /** @brief Espera a que el hilo del Interconnect termine. */
    void join_interconnect_thread();

/* --- */

    /** @brief Devuelve true si TODOS los PEs están en estado FINISHED. */
    bool all_pes_finished() const;

};
