#ifndef LOCAL_CACHE_H
#define LOCAL_CACHE_H

#include <vector>
#include <array>
#include <cstdint>
#include <string>

/**
 * @class LocalCache
 * @brief Representa un caché privado de un Processing Element (PE).
 *
 * Este caché consta de un número fijo de bloques (BLOCKS),
 * cada uno de un tamaño fijo en bytes (BLOCK_SIZE). Proporciona
 * métodos para inicializar su contenido con datos aleatorios y
 * volcar su estado a un archivo de texto para depuración.
 */
class LocalCache {
public:
    static constexpr size_t BLOCKS = 128;       /**< Numero de bloques en el cache. */
    static constexpr size_t BLOCK_SIZE = 16;    /**< Tamaño de cada bloque en bytes. */

    /**
     * @brief Construye un caché vacío con el número de bloques definido.
     */
    LocalCache(int id);

/* ---------------------------------------- Initializing --------------------------------------- */

    /**
     * @brief Inicializa el cache llenandolo de forma aleatoria y lo vuelca.
     */
    void initialize();

    /**
     * @brief Rellena el caché con datos aleatorios.
     *
     * Genera valores aleatorios de 0 a 255 para cada byte
     * de cada bloque en el caché, usando un generador
     * Mersenne Twister para asegurar calidad de entropía.
     */
    void fill_random();

    /**
     * @brief Vuelca el contenido del caché a un archivo de texto.
     *
     * Cada línea representará un bloque completo, mostrando
     * sus bytes en notación binaria (8 bits por byte).
     * Si la carpeta del archivo no existe, se crea automáticamente.
     *
     * @param output_filename Ruta completa del archivo de salida.
     * @throws std::runtime_error Si no se puede crear o escribir en el archivo.
     */
    void dump_to_text_file() const;

/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------- Data Handling --------------------------------------- */ 

    /**
     * @brief Lee un bloque de líneas de caché desde el fichero de volcado.
     *
     * Abre el archivo de texto correspondiente al caché del PE indicado
     * (../config/caches/cache_<id>.txt), salta hasta la línea de inicio
     * especificada y lee el número de líneas indicado, parseando cada línea
     * como bytes en notación hexadecimal.
     *
     * Admite dos formatos de línea:
     *  - Tokens hexadecimales separados por espacios (p. ej. "ab cd ef …").
     *  - Cadena continua de dígitos hexadecimales (p. ej. "c80b…").
     *
     * @param id         Identificador del PE (se corresponde con el nombre de fichero).
     * @param start_line Índice (0-based) de la primera línea de caché a leer.
     * @param num_lines  Número de líneas consecutivas que se desean leer.
     * @return Vector de tamaño `num_lines`, donde cada entrada es un vector de
     *         `BLOCK_SIZE` bytes leídos del caché.
     *
     * @throws std::runtime_error   Si no se puede abrir el archivo o si el formato
     *                              de alguna línea es inválido.
     * @throws std::out_of_range    Si `start_line + ln` excede el número de líneas
     *                              disponibles en el fichero.
     */
    static std::vector<std::vector<uint8_t>> read_cache_from_file(uint32_t id, 
                                                                  uint32_t start_line, 
                                                                  uint32_t num_lines);

    /**
     * @brief Sobrescribe una o más líneas en el fichero de caché del PE.
     *
     * Lee todo el archivo de volcado de caché correspondiente al PE indicado,
     * reemplaza las líneas a partir de `start_line` con los bytes proporcionados
     * en `lines`, y luego escribe de nuevo todo el contenido al disco.
     *
     * @param id          Identificador del PE (se usa para construir el nombre
     *                    de fichero "cache_<id>.txt").
     * @param start_line  Índice (0-based) de la primera línea a sobrescribir.
     * @param lines       Vector de vectores de bytes, donde cada sub-vector
     *                    representa exactamente BLOCK_SIZE bytes que reemplazarán
     *                    a la línea correspondiente en el fichero.
     *
     * @throws std::runtime_error      Si no se puede abrir o escribir el fichero.
     * @throws std::out_of_range      Si `start_line` o `start_line + lines.size()`
     *                                exceden el número de líneas existentes.
     * @throws std::invalid_argument  Si alguna de las entradas en `lines` no tiene
     *                                exactamente BLOCK_SIZE bytes.
     */
    static void write_cache_lines(uint32_t id, uint32_t start_line,
                                    const std::vector<std::vector<uint8_t>>& lines);

    static void invalidate_line(uint32_t line_index, int pe_id);

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

    /**
     * @brief Simula la lectura de varias líneas de caché.
     *
     * Imprime en consola el contenido de cada byte en hexadecimal
     * para las líneas [start_line, start_line+num_of_lines).
     *
     * @param start_line Índice de la primera línea a leer (0-based).
     * @param num_of_lines Cantidad de líneas a leer.
     * @throws std::out_of_range Si el rango excede BLOCKS.
     */
    void read_test(uint32_t start_line, uint32_t num_lines) const;

/* --------------------------------------------------------------------------------------------- */

private:
    int id_;                            /**< ID del PE al que pertenece. */
    std::string dump_path;              /**< Directorio donde se volcara el cache. */
    std::string inv_path;
    /// Número de bloques en el caché.
    //static constexpr size_t BLOCKS = 128;
    /// Tamaño de cada bloque en bytes.
    // static constexpr size_t BLOCK_SIZE = 16;
    /// Datos del caché: vector de bloques, cada uno es un array de bytes.

    std::vector<std::array<uint8_t, BLOCK_SIZE>> cache_data; /**< vector de bloques, cada uno es un array de bytes. */
};

#endif // LOCAL_CACHE_H
