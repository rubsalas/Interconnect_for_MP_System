/**
 * @file main.cpp
 * @brief Simulador de Interconnect para Sistemas Multiprocesador.
 *
 * Este programa permite inicializar el sistema, ejecutar la simulación
 * y mostrar estadísticas de los PEs y del Interconnect.
 *
 */

 #include "../include/Compiler.h"
 #include "../include/System.h"
 #include "../include/Instruction_Generator.h" // <-- AGREGADO
 
 #include <iostream>
 #include <string>
 #include <limits>
 
 constexpr int MAX_PES = 32;          /**< Límite máximo de PEs según especificación */
 int pe_count = 0;                    /**< Cantidad de PEs configurada por el usuario */
 
 // instancia global del Compiler
 static Compiler* compiler = nullptr;
 
 // Instancia global del System
 static System* interconnect_system = nullptr;
 
 // Forward declarations
 void show_menu();
 void compile_instructions();
 void initialize_system();
 void run_simulation();
 void show_statistics();
 void generate_instruction_files(); // <-- AGREGADO
 
 /**
  * @brief Punto de entrada de la aplicación.
  *
  * @return int Código de estado de la aplicación (0 indica éxito).
  */
 int main() {
     bool running = true;
     int choice = -1;
 
     std::cout << "=== Interconnect for MP Systems Simulator ===\n";
     while (running) {
         show_menu();
         std::cout << "Select option: ";
 
         if (!(std::cin >> choice)) {
             // Limpia entrada inválida
             std::cin.clear();
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
             std::cout << "[Error] Invalid input. Please enter a number.\n";
             continue;
         }
 
         switch (choice) {
             case 1:
                 compile_instructions();
                 break;
             case 2:
                 initialize_system();
                 break;
             case 3:
                 run_simulation();
                 break;
             case 4:
                 show_statistics();
                 break;
             case 5: // <-- AGREGADO
                 generate_instruction_files();
                 break;
             case 0:
                 std::cout << "\nExiting program...\n";
                 running = false;
                 break;
             default:
                 std::cout << "[Error] Option not recognized.\n";
         }
     }
 
     std::cout << "Program terminated.\n";
     return 0;
 }
 
 // ---------------- Helper Function Definitions ----------------
 
 /**
  * @brief Implementación del menú de opciones.
  */
 void show_menu() {
     std::cout << "\n--- Main Menu ---\n"
               << "1. Compile Assembly\n"
               << "2. Initialize System\n"
               << "3. Run Simulation\n"
               << "4. Show Statistics\n"
               << "5. Generate Instruction Files\n" // <-- AGREGADO
               << "0. Exit\n";
 }
 
 /**
  * @brief Obtiene las instrucciones y las pasa a un binario que entienden
  * los PEs.
  */
 /// @brief Invoca al compilador de directorios con rutas fijas por ahora.
 void compile_instructions() {
     std::cout << "\n[Init] Compiling assembly...\n";
     // Paths
     const std::string in_dir  = "config/assemblers";
     const std::string out_dir = "config/binaries";
     compiler->compile_directory(in_dir, out_dir);
     std::cout << "[Init] Binary ready for execution on PEs.\n";
 }
 
 /**
  * @brief Configura el sistema según la cantidad de PEs indicada.
  *
  * Realiza la validación de entrada y despliega pasos de inicialización.
  */
 void initialize_system() {
     int requested_pes = 0;
 
     // Usuario escoge la cantidad de PEs por utilizar en la simulacion
     std::cout << "\n[Init] Enter number of Processing Elements (1-" << MAX_PES << "): ";
     while (true) {
         if (!(std::cin >> requested_pes)) {
             std::cin.clear();
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
             std::cout << "[Error] Invalid input. Please enter an integer between 1 and " << MAX_PES << ": ";
             continue;
         }
         if (requested_pes < 1 || requested_pes > MAX_PES) {
             std::cout << "[Error] Value out of range. Please enter a number between 1 and " << MAX_PES << ": ";
         } else {
             break;
         }
     }
 
     pe_count = requested_pes;
     std::cout << "[Init] System configured with " << pe_count << " PEs.\n";
 
     // Inicializa la instancia global
     interconnect_system = new System(pe_count);
 
     // Llama la funcion para inicializar el System
     interconnect_system->initialize();
 }
 
 /**
  * @brief Simula un número fijo de ciclos de operación.
  * TODO
  */
 void run_simulation() {
     if (pe_count == 0 || !interconnect_system) {
         std::cout << "\n[Sim] System not initialized. Please initialize first.\n";
         return;
     }
 
     std::cout << "\n[Sim] Starting simulation with " << pe_count << " PEs...\n";
 
     interconnect_system->run();
 }
 
 /**
  * @brief Muestra estadísticas almacenadas tras la simulación.
  * TODO
  */
 void show_statistics() {
     if (pe_count == 0 || !interconnect_system) {
         std::cout << "\n[Stats] No statistics available: system not initialized.\n";
         return;
     }
 
     interconnect_system->report_statistics();
 }
 
 /**
  * @brief Genera archivos de instrucciones para los PEs.
  */
 void generate_instruction_files() {
     int requested_pes = 0;
 
     std::cout << "\n[Init] Enter number of Processing Elements to generate instructions for (1-" << MAX_PES << "): ";
     while (true) {
         if (!(std::cin >> requested_pes)) {
             std::cin.clear();
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
             std::cout << "[Error] Invalid input. Please enter an integer between 1 and " << MAX_PES << ": ";
             continue;
         }
         if (requested_pes < 1 || requested_pes > MAX_PES) {
             std::cout << "[Error] Value out of range. Please enter a number between 1 and " << MAX_PES << ": ";
         } else {
             break;
         }
     }
 
     InstructionGenerator generator(requested_pes);
     generator.generate();
     std::cout << "[Init] Instruction files generated for " << requested_pes << " PEs.\n";
 }
 