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
#include "../include/Instruction_Generator.h"

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
void define_PEs();
void show_menu();
void compile_instructions();
void initialize_system();
void run_simulation();
void show_statistics();
void generate_instruction_files();
// Test declarations
void test_G();
void test_R();

/**
 * @brief Punto de entrada de la aplicación.
 *
 * @return int Código de estado de la aplicación (0 indica éxito).
 */
int main() {
	bool running = true;
	int choice = -1;

	std::cout << "=== Interconnect for MP Systems Simulator ===\n";

	define_PEs();

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
				generate_instruction_files();
				break;
			case 2:
				compile_instructions();
				break;
			case 3:
				initialize_system();
				break;
			case 4:
				run_simulation();
				break;
			case 5:
				show_statistics();
				break;
			case 8:
				test_G();
				break;
			case 9:
				test_R();
				break;
			case -1:
				define_PEs();
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

/**
 * @brief Configura la cantidad de PEs que se usaran en la simulacion.
 */
void define_PEs() {
	int requested_pes = 0;

    // Selección de número de PEs
    std::cout << "\n[Init] Enter number of Processing Elements (8-" << MAX_PES << "): ";
    while (!(std::cin >> requested_pes) ||
           requested_pes < 1 || requested_pes > MAX_PES) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[Error] Please enter an integer between 8 and "
                  << MAX_PES << ": ";
    }
    pe_count = requested_pes;
    std::cout << "[Init] System configured with " << pe_count << " PEs.\n";
}

/**
 * @brief Implementación del menú de opciones.
 */
void show_menu() {
	std::cout << "\n--- Main Menu ---\n"
			<< "1. Generate Instruction Files\n"
			<< "2. Compile Assembly\n"
			<< "3. Initialize System\n"	// <-- MODIFICADO
			<< "4. Run Simulation\n"
			<< "5. Show Statistics\n"
			<< "8. Test G\n"
			<< "9. Test R\n"
			<< "0. Exit\n";
}

/**
 * @brief Genera archivos de instrucciones para los PEs.
 */
void generate_instruction_files() {
	if (pe_count == 0 ) {
		std::cout << "\n[Sim] Program initialized with no PEs. Please initialize correctly.\n";
		return;
	}

	InstructionGenerator generator(pe_count);
	generator.generate();
	std::cout << "[Init] Instruction files generated for " << pe_count << " PEs.\n";
}

/**
 * @brief Obtiene las instrucciones y las pasa a un binario que entienden
 * los PEs.
 */
void compile_instructions() {
	std::cout << "\n[Init] Compiling assembly...\n";
	// Paths
	const std::string in_dir  = "config/assemblers";
	const std::string out_dir = "config/binaries";
	compiler->compile_directory(in_dir, out_dir);
	std::cout << "[Init] Binary ready for execution on PEs.\n";
}

/**
 * @brief Configura el sistema según la cantidad de PEs y esquema de arbitraje indicados.
 */
void initialize_system() {
    int scheme_choice = 0;

    // 1) Selección del esquema de arbitraje
    std::cout << "\n[Init] Select Interconnect arbitration scheme:\n"
              << "  1. FIFO\n"
              << "  2. PRIORITY (based on QoS)\n"
              << "Choice (1-2): ";
    while (!(std::cin >> scheme_choice) ||
           (scheme_choice != 1 && scheme_choice != 2)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[Error] Enter 1 for FIFO or 2 for PRIORITY: ";
    }

    // 2) Instanciación y configuración del System
    ArbitScheme scheme = (scheme_choice == 1)
        ? ArbitScheme::FIFO
        : ArbitScheme::PRIORITY;
    interconnect_system = new System(pe_count, scheme);
    interconnect_system->initialize();

    std::cout << "[Init] Initialization complete with "
              << (scheme == ArbitScheme::FIFO ? "FIFO" : "PRIORITY") << " arbitration scheme.\n";
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

/* ---------------------------------------- Testing -------------------------------------------- */

void test_G() {
	std::cout << "\n[TEST] Starting Test G...\n";
	std::string file_path;
    std::cout << "\n[Test] Enter path to instruction dump file: ";
    std::cin >> file_path;
	interconnect_system->system_test_G(file_path);
	
	//std::vector<Message> mensajes = interconnect_system->system_test_G(file_path);

	std::cout << "[System] Mensajes decodificados:\n";
   
}

void test_R() {
	std::cout << "\n[TEST] Starting Test R...\n";
	interconnect_system->system_test_R();
}

/* --------------------------------------------------------------------------------------------- */
