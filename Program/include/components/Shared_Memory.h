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

/* ---------------------------------------- Initializing --------------------------------------- */

    /**
     * @brief Inicializa el shared memory llenandolo de forma aleatoria y lo vuelca.
     */
    void initialize();

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

/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------- Data Handling --------------------------------------- */ 

    /**
     * @brief Sobrescribe múltiples bloques en la representación de texto de la memoria compartida.
     *
     * Lee el fichero de volcado de memoria compartida (dump_path_txt), reemplaza las líneas
     * correspondientes comenzando en la dirección de palabra @p address y luego escribe de nuevo
     * todo el contenido al disco. Cada bloque en @p blocks debe contener exactamente 16 bytes
     * (4 palabras de 4 bytes), y se mapea a 4 líneas consecutivas en el fichero, cada línea
     * representando una palabra en notación hexadecimal de 8 dígitos.
     *
     * @param blocks  Vector de bloques a escribir; cada bloque es un vector de 16 bytes.
     * @param address Índice de palabra (0-based) desde el cual comenzar la escritura. Cada
     *                bloque ocupa cuatro líneas, por lo que se sobrescriben las líneas
     *                @p address ... @p address + 4*blocks.size() - 1.
     *
     * @note En caso de fallo al abrir el fichero, bloque de tamaño incorrecto o
     *       dirección fuera de rango, se imprimirá un mensaje de error a std::cerr
     *       y la función retornará prematuramente sin lanzar excepciones.
     */
    void write_shared_memory_lines(const std::vector<std::vector<uint8_t>>& blocks,
                                size_t address);
    
    /*
    * @brief Lee bloques de palabra desde la representación de texto de la memoria compartida.
    *
    * Abre el fichero de volcado de memoria compartida (dump_path_txt) y retorna las líneas
    * correspondientes al rango especificado por @p address y @p size_bytes. Cada línea en
    * el fichero representa una palabra de 4 bytes en hexadecimal de 8 dígitos.
    *
    * La función agrupa las palabras en bloques de 4 líneas (128 bits):  
    * - Calcula cuántas líneas (palabras) son necesarias para cubrir @p size_bytes.  
    * - Agrupa esas líneas en sub-vectores de longitud 4 (padding con "00000000" si el fichero \ 
    *   no tiene suficientes líneas).  
    *
    * @param address    Índice de palabra (0-based) desde el cual iniciar la lectura.  
    * @param size_bytes Número total de bytes a leer; redondea hacia arriba al siguiente \ 
    *                   múltiplo de 4.  
    * @return Vector de bloques, donde cada bloque es un vector de cuatro cadenas de 8 dígitos \ 
    *         hexadecimales. Si la lectura excede el final del fichero, las palabras faltantes \ 
    *         se rellenan con "00000000".  
    *
    * @note En caso de error al abrir el fichero, se imprime un mensaje de error y se retorna \ 
    *       un vector vacío.
    */
std::vector<std::vector<std::uint8_t>> read_shared_memory(size_t address,
                                                            size_t size_bytes);

/* --------------------------------------------------------------------------------------------- */

private:
    static constexpr size_t     MEMORY_SIZE = 4096; /**< Número de palabras de 32 bits en la memoria. */
    std::vector<uint32_t>       data;               /**< Contenedor interno de las palabras de memoria. */
    std::string dump_path_txt;                      /**< Directorio donde se volcara el shared memory. */
    std::string dump_path_bin;                      /**< Directorio donde se volcara el shared memory. */
};

#endif // SHARED_MEMORY_H
