#ifndef INSTRUCTION_MEMORY_H
#define INSTRUCTION_MEMORY_H

#include <vector>
#include <cstdint>
#include <string>

/**
 * @class InstructionMemory
 * @brief Gestiona la memoria de instrucciones de un PE.
 *
 * Cada instancia está asociada a un PE mediante un identificador. Permite
 * cargar instrucciones de 64 bits desde un archivo de texto (hex o binario),
 * obtener instrucciones por índice y volcar la memoria a un archivo.
 */
class InstructionMemory {
public:
    /**
     * @brief Construye una memoria de instrucciones para un PE.
     * @param pe_id Identificador del PE asociado.
     */
    explicit InstructionMemory(int pe_id);

    /**
     * @brief Inicializa la memoria a partir de un archivo, imprime y vuelca.
     *
     * - Carga las instrucciones del archivo.
     * - Las muestra por consola en hex y binario.
     * - Vuelca a "dump_memoria_PE<pe_id>.txt".
     *
     * @param filename Ruta al archivo de texto con instrucciones.
     */
    void initialize();

    /**
     * @brief Carga instrucciones desde un archivo de texto.
     *
     * Cada línea no vacía se interpreta como:
     * - Hexadecimal con prefijo "0x".
     * - Binario (solo '0' y '1'), máximo 64 caracteres.
     * Instrucciones que excedan 43 bits lanzan excepción.
     *
     * @param filename Ruta al archivo de texto.
     * @throws std::runtime_error si no puede abrir el archivo.
     * @throws std::invalid_argument o std::overflow_error según parseo.
     */
    void load_from_file(const std::string& filename);

    /**
     * @brief Recupera la instrucción en la posición indicada.
     * @param address Índice (0-based) de la instrucción.
     * @return Valor de 64 bits de la instrucción.
     * @throws std::out_of_range si la dirección está fuera de rango.
     */
    uint64_t fetch_instruction(size_t address) const;

    /**
     * @brief Devuelve el número de instrucciones almacenadas.
     * @return Cantidad de instrucciones.
     */
    size_t size() const;

    /**
     * @brief Vuelca el contenido de la memoria a un archivo de texto.
     *
     * Cada instrucción se escribe como una línea de 64 bits en formato binario.
     *
     * @param output_filename Ruta del archivo de salida.
     * @throws std::runtime_error si no puede crear el archivo.
     */
    void dump_to_file(const std::string& output_filename) const;

private:
    int pe_id_;                          /**< ID del PE asociado a esta memoria. */
    std::string binary_path;             /**< Directorio donde se encuentra el binario. */
    std::vector<uint64_t> instructions;  /**< Vector interno de instrucciones. */

    /**
     * @brief Parsea una línea de texto a un valor de 64 bits.
     *
     * Soporta hexadecimal (con prefijo "0x") y binario puro.
     *
     * @param text Cadena con la instrucción.
     * @return Valor numérico de la instrucción.
     * @throws std::invalid_argument o std::overflow_error según el formato.
     */
    uint64_t parse_instruction(const std::string& text);
};

#endif // INSTRUCTION_MEMORY_H
