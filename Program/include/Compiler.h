// include/Compiler.h
#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <fstream>

/**
 * @class Compiler
 * @brief Convierte instrucciones de ensamblador en binario y administra compilación en lote.
 */
class Compiler {
public:
    /**
     * @brief Convierte un valor entero a una cadena binaria de longitud fija.
     * @param value Valor entero a convertir.
     * @param bits  Número de bits deseado.
     * @return Cadena binaria.
     */
    static std::string to_bin(int value, int bits);

    /**
     * @brief Valida y convierte el campo SRC a binario.
     * @param value Cadena con el valor a validar.
     * @return Cadena binaria de tamaño SRC.
     */
    static std::string validate_src(const std::string& value);

    /**
     * @brief Valida y convierte la dirección a binario.
     * @param value Cadena con el valor a validar.
     * @return Cadena binaria de tamaño ADDR.
     */
    static std::string validate_addr(const std::string& value);

    /**
     * @brief Valida y convierte un número de línea de caché a binario.
     * @param value Cadena con el valor a validar.
     * @return Cadena binaria de tamaño CACHE_LINE.
     */
    static std::string validate_cache_line(const std::string& value);

    /**
     * @brief Valida y convierte el valor de QoS a binario.
     * @param value Cadena con el valor a validar.
     * @return Cadena binaria de tamaño QoS.
     */
    static std::string validate_qos(const std::string& value);

    /**
     * @brief Limpia y tokeniza instrucciones desde un flujo de entrada.
     * @param file Flujo de archivo de entrada.
     * @return Vector de instrucciones tokenizadas.
     */
    static std::vector<std::vector<std::string>> clean_instructions(std::ifstream& file);

    /**
     * @brief Convierte instrucciones tokenizadas a su representación binaria.
     * @param instructions Vector de instrucciones tokenizadas.
     * @return Vector de cadenas binarias.
     */
    static std::vector<std::string> get_binary(const std::vector<std::vector<std::string>>& instructions);

    /**
     * @brief Compila todos los archivos de instrucciones en `input_dir` y genera `.bin` en `output_dir`.
     * @param input_dir  Directorio de archivos de entrada.
     * @param output_dir Directorio donde colocar los binarios.
     */
    void compile_directory(const std::string& input_dir,
                           const std::string& output_dir) const;
};

#endif // COMPILER_H
