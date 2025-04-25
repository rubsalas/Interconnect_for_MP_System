#pragma once

#include <vector>
#include "components/PE.h"

/**
 * @class System
 * @brief Controlador principal de la simulación MP: gestiona PEs, ciclos y estadísticas.
 */
class System {
public:
    /**
     * @brief Construye un sistema con un número dado de PEs.
     * @param num_pes Cantidad de processing elements a instanciar.
     */
    explicit System(int num_pes);

    /** @brief Inicializa componentes internos (PEs, interconnect, memoria, estadísticas). */
    void initialize();

    /** @brief Ejecuta la simulación. */
    void run();

    /** @brief Reporta estadísticas finales tras la simulación. */
    void report_statistics() const;

    /**
     * @brief Imprime en consola el ID y QoS de cada PE para depuración.
     */
    void debug_print_pes() const;

private:
    std::vector<PE> pes_;        /**< Vector de PEs del sistema. */
    int total_pes_;              /**< Número de PEs configurados. */

    /**
     * @brief Crea e inicializa los PEs con QoS por defecto.
     *
     * Más adelante, cargará los valores de QoS desde un archivo de configuración.
     */
    void initialize_pes();
};
