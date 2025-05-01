#pragma once

#include <vector>
#include <memory>
#include "components/PE.h"
#include "components/Interconnect.h"

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

private:
    std::vector<PE>                    pes_;            /**< Vector de PEs del sistema. */
    int                                total_pes_;      /**< Número de PEs configurados. */
    std::unique_ptr<Interconnect>      interconnect_;   /**< Interconnect para enrutar mensajes. */
    ArbitScheme                        scheme_;         /**< Esquema de arbitraje seleccionado. */

    /**
     * @brief Crea e inicializa los PEs con QoS por defecto.
     *
     * Más adelante, cargará los valores de QoS desde un archivo de configuración.
     */
    void initialize_pes();
};
