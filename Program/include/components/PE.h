#pragma once

#include <cstdint>

/**
 * @class PE
 * @brief Processing Element minimal con QoS.
 */
class PE {
public:
    /**
     * @brief Construye un PE con identificador y QoS.
     * @param id  Identificador único del PE.
     * @param qos Calidad de servicio (0x00–0xFF).
     */
    PE(int id, uint8_t qos);

    /** @brief Devuelve el identificador del PE. */
    int get_id() const;
    /** @brief Devuelve el valor de QoS asignado. */
    uint8_t get_qos() const;

private:
    int     id_;   /**< ID del PE. */
    uint8_t qos_;  /**< Calidad de servicio. */
};
