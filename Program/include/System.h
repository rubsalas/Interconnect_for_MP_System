#pragma once

#include <vector>
#include <memory>
#include <thread>
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
    System(int num_pes, ArbitScheme scheme);

    /** @brief Inicializa PEs, caches, interconnect y memoria compartida. */
    void initialize();

    /** @brief Ejecuta la simulación. */
    void run();

    /**
     * @brief Lanza un hilo para el PE en la posición `pe_id` del vector pes_.
     *
     * Cada hilo ejecutará `pe_worker(pe_id)` y quedará registrado en pe_threads_.
     *
     * @param pe_id Índice del PE en pes_ (de 0 a total_pes_-1).
     */
    void start_pe_thread(int pe_id);

    /** @brief Espera a que todos los hilos de PE terminen. */
    void join_pe_threads();

    /** @brief Reporta estadísticas finales tras la simulación. */
    void report_statistics() const;

    /**
     * @brief Imprime en consola el ID y QoS de cada PE para depuración.
     */
    void debug_print() const;

    void system_test_G();
    void system_test_R();

private:
    int                             total_pes_;     /**< Número de PEs configurados. */
    std::vector<PE>                 pes_;           /**< Vector de PEs del sistema. */
    ArbitScheme                     scheme_;        /**< Esquema de arbitraje seleccionado. */
    std::unique_ptr<Interconnect>   interconnect_;  /**< Interconnect para enrutar mensajes. */
    std::vector<LocalCache>         caches_;        /**< Caches Locales L1 para cada PE. */
    std::unique_ptr<SharedMemory>   shared_memory_; /**< Interconnect para enrutar mensajes. */

    // --------------------------------------------------------------------
    // Sub­sistema de threading
    // --------------------------------------------------------------------
    std::vector<std::thread> pe_threads_; /**< Hilos que ejecutan cada PE. */

    /**
     * @brief Crea e instancia el Interconnect como unico
     */
    void initialize_interconnect();
    
    /**
     * @brief Crea e inicializa los PEs con QoS por defecto.
     */
    void initialize_pes();

    /**
     * @brief Inicializa un caché local por cada PE (sin QoS).
     */
    void initialize_caches();

    /** 
     * @brief Inicializa la memoria compartida del sistema. 
     */
    void initialize_shared_memory();

    /**
     * @brief Rutina que corre dentro de cada hilo de PE.
     *
     * Aquí es donde cada PE hará su ciclo de fetch/execute,
     * enviará mensajes al interconect y recibirá respuestas.
     *
     * @param pe_id Índice del PE en el vector pes_.
     */
    void pe_execution_cycle(int pe_id);
};
