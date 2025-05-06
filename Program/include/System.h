#pragma once

#include <vector>
#include <memory>
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

    /** @brief Inicializa componentes internos (PEs, interconnect, memoria, estadísticas). */
    void initialize();

    /** @brief Ejecuta la simulación. */
    void run();

    /** @brief Reporta estadísticas finales tras la simulación. */
    void report_statistics() const;

    /**
     * @brief Imprime en consola el ID y QoS de cada PE para depuración.
     */
    void debug_print() const;

    void system_test_G(const std::string& file_path);
    void system_test_R();

private:
    int                             total_pes_;     /**< Número de PEs configurados. */
    std::vector<PE>                 pes_;           /**< Vector de PEs del sistema. */
    ArbitScheme                     scheme_;        /**< Esquema de arbitraje seleccionado. */
    std::unique_ptr<Interconnect>   interconnect_;  /**< Interconnect para enrutar mensajes. */
    std::vector<LocalCache>         caches_;        /**< Caches Locales L1 para cada PE. */
    std::unique_ptr<SharedMemory>   shared_memory_; /**< Interconnect para enrutar mensajes. */  

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
};
