#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>

/**
 * @class SharedMemory
 * @brief Simula la memoria principal compartida de un sistema multiprocesador.
 *
 * Cada palabra de memoria tiene 32 bits, y la memoria completa consta de un
 * número fijo de posiciones (MEMORY_SIZE). Se proveen métodos para lectura
 * y escritura, inicialización aleatoria y volcado de contenido a archivos
 * tanto binarios como de texto (representación en bits).
 */
class SharedMemory {
public:
    /**
     * @brief Construye la memoria e inicializa todas las posiciones a cero.
     */
    SharedMemory();

    /**
     * @brief Inicializa el shared memory llenandolo de forma aleatoria y lo vuelca.
     */
    void initialize();

    /**
     * @brief Lee una palabra de 32 bits en la dirección especificada.
     * @param address Índice de la palabra a leer (0-based).
     * @return Valor de 32 bits almacenado en la posición.
     * @throws std::out_of_range Si la dirección está fuera del rango válido.
     */
    uint32_t load(size_t address) const;

    /**
     * @brief Escribe un valor de 32 bits en la dirección especificada.
     * @param address Índice de la palabra a escribir (0-based).
     * @param value Valor de 32 bits a almacenar.
     * @throws std::out_of_range Si la dirección está fuera del rango válido.
     */
    void store(size_t address, uint32_t value);

    /**
     * @brief Devuelve la capacidad total de la memoria en palabras de 32 bits.
     * @return Número total de posiciones (MEMORY_SIZE).
     */
    size_t size() const;

    /**
     * @brief Rellena toda la memoria con valores aleatorios de 32 bits.
     *
     * Se utiliza un motor Mersenne Twister y una distribución uniforme
     * en el rango [0, UINT32_MAX].
     */
    void fill_random();

    /**
     * @brief Vuelca el contenido de la memoria a un archivo binario.
     *
     * Escribe directamente el array de memoria como datos binarios.
     * @param output_filename Ruta al archivo binario de salida.
     * @throws std::runtime_error Si no se puede crear o escribir en el archivo.
     */
    void dump_to_binary_file() const;

    /**
     * @brief Vuelca el contenido de la memoria a un archivo de texto.
     *
     * Cada línea del archivo contendrá la representación en 32 bits
     * ('0'/'1') de cada palabra de memoria.
     * @param output_filename Ruta al archivo de texto de salida.
     * @throws std::runtime_error Si no se puede crear o escribir en el archivo.
     */
    void dump_to_text_file() const;

private:
    static constexpr size_t     MEMORY_SIZE = 4096; /**< Número de palabras de 32 bits en la memoria. */
    std::vector<uint32_t>       data;               /**< Contenedor interno de las palabras de memoria. */
    std::string dump_path_txt;                      /**< Directorio donde se volcara el shared memory. */
    std::string dump_path_bin;                      /**< Directorio donde se volcara el shared memory. */
};

#endif // SHARED_MEMORY_H
