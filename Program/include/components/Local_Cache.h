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
    /**
     * @brief Construye un caché vacío con el número de bloques definido.
     */
    LocalCache(int id);

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

private:
    int id_;                            /**< ID del PE al que pertenece. */
    std::string dump_path;              /**< Directorio donde se volcara el cache. */
    /// Número de bloques en el caché.
    static constexpr size_t BLOCKS = 128;
    /// Tamaño de cada bloque en bytes.
    static constexpr size_t BLOCK_SIZE = 16;
    /// Datos del caché: vector de bloques, cada uno es un array de bytes.
    std::vector<std::array<uint8_t, BLOCK_SIZE>> cache_data;
};

#endif // LOCAL_CACHE_H
