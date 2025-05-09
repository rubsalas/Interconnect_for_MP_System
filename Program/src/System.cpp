#include "../include/System.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <thread>
#include <bitset>

/* ---------------------------------------- Constructor ---------------------------------------- */

System::System(int num_pes, ArbitScheme scheme)
    : total_pes_(num_pes), scheme_(scheme) {
    pes_.reserve(total_pes_);
    std::cout << "\n[System] Created with " << total_pes_ << " PEs and "
              << (scheme_ == ArbitScheme::FIFO ? "FIFO" : "PRIORITY") << " scheme.\n";   
}

/* --------------------------------------------------------------------------------------------- */

/* -------------------------------------- Initialization --------------------------------------- */

void System::initialize() {

    std::cout << "\n[System] Initializing Interconnect...\n";
    initialize_interconnect();
    
    std::cout << "\n[System] Initializing " << total_pes_ << " PEs...\n";
    initialize_pes();

    std::cout << "\n[System] Setting up Local Cache for " << total_pes_ << " PEs...\n";
    initialize_caches();

    std::cout << "\n[System] Setting up Shared Memory...\n";
    initialize_shared_memory();

    // TODO: estas inicializaciones
    std::cout << "\n[System] Getting simulation times... (TODO)\n";

    std::cout << "\n[System] Setting up Statistics Unit... (TODO)\n";

    std::cout << "\n[System] Initialization complete.\n";
}

void System::initialize_interconnect() {
    interconnect_ = std::make_unique<Interconnect>(total_pes_, scheme_);
}

void System::initialize_pes() {
    std::cout << "[System] Getting PEs' QoS from config/qos.txt...\n";
    // Mapa id -> QoS
    std::unordered_map<int, uint8_t> qos_map;
    std::ifstream infile("config/qos.txt");
    if (!infile) {
        std::cerr << "[System] Warning: could not open config/qos.txt, "
                  << "using default QoS=0 for all PEs\n";
    } else {
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            int id; char colon; std::string hexval;
            if (!(iss >> id >> colon >> hexval)) continue;
            try {
                uint8_t val = static_cast<uint8_t>(
                    std::stoul(hexval, nullptr, 16)
                );
                qos_map[id] = val;
            } catch (...) {
                std::cerr << "[System] Warning: invalid QoS entry: " << line << "\n";
            }
        }
    }

    // Instancia los PEs con QoS leído o 0 por defecto
    pes_.clear();
    for (int i = 0; i < total_pes_; ++i) {
        uint8_t qos = qos_map.count(i) ? qos_map[i] : 0;
        pes_.emplace_back(i, qos);
    }
}

void System::initialize_caches() {
    caches_.clear();
    caches_.reserve(total_pes_);
    for (int i = 0; i < total_pes_; ++i) {
        caches_.emplace_back(i);  // construye un LocalCache vacío
        std::cout << "[System] Cache " << i << " instantiated.\n";
    }

    std::cout << "[System] All local caches initialized.\n";
}

void System::initialize_shared_memory() {
    shared_memory_ = std::make_unique<SharedMemory>();
}

/* --------------------------------------------------------------------------------------------- */

/* ----------------------------------------- Execution ----------------------------------------- */

void System::step() {
    {
        // Bloqueamos el mutex solo el tiempo de incrementar
        std::lock_guard<std::mutex> lk(step_mtx_);
        ++current_step_;
    }
    // Despertamos a todos: cada hilo que esté esperando en step_cv_
    step_cv_.notify_all();
}

void System::run() {

    // 1) Lanzar hilos para cada PE
    for (int i = 0; i < total_pes_; ++i) {
        start_pe_thread(i);
    }

    // 2) Lanzar hilo del Interconnect
    start_interconnect_thread();

    // 3) Bucle de stepping: esperar Enter y avanzar un ciclo
    std::string line;
    while (!all_pes_finished() || interconnect_->get_state() != ICState::FINISHED) {
        // Pedir al usuario que avance
        std::cout << "\nPRESS [Enter] TO ADVANCE ONE CYCLE...\n\n";
        std::getline(std::cin, line);

        // Disparar un nuevo paso
        step();
    }

    // 4) Esperar a que todos los PEs terminen
    join_pe_threads();

    // 5) Esperar a que el Interconnect termine
    join_interconnect_thread();
}

/* ------------------------------------ */
/*                                      */
/*             PE's threads             */
/*                                      */
/* ------------------------------------ */

void System::start_pe_thread(int pe_id) {
    // Validación básica del índice
    if (pe_id < 0 || pe_id >= total_pes_) {
        std::cerr << "[System] Error: PE id out of range: " << pe_id << "\n";
        return;
    }

    // Construye un std::thread que ejecuta el método de instancia pe_execution_cycle(pe_id)
    // std::thread toma como primer argumento un puntero a miembro, seguido de 'this'
    // y finalmente el argumento pe_id.
    pe_threads_.emplace_back(&System::pe_execution_cycle, this, pe_id);

    std::cout << "[System] Launching thread for PE " << pe_id << "...\n";
}

void System::pe_execution_cycle(int pe_id) {

    /**/
    int last_step = 0;

    // 0.1) Referencia al PE correspondiente
    PE& pe = pes_.at(pe_id);

    // 0.2) Referencia al Cache correspondiente al PE
    LocalCache& cache = caches_.at(pe_id);

    // 0.3) Se obtiene la cantidad de instrucciones por ejecutar
    size_t total_instr = pe.instruction_memory_.size();

    // 0.4) Mensaje de arranque del hilo para este PE
    std::cout << "[PE " << pe.get_id() << "] Thread starting "
              << ", QoS=0x" << std::hex << int(pe.get_qos())
              << std::dec
              << ", state=" << pe.state_to_string()
              << ", instr_count=" << total_instr
              << "\n";

    // 0.3) Mostrar instrucciones por correr
    //std::cout << "[PE Worker] Showing instructions that will be executed by PE " << pe.get_id() << "\n" ;
    //pe.instruction_memory_.print_instructions();

    /* Ciclo de ejecucion correra hasta que el estado del PE llegue a FINISHED */
    while (pe.get_state() != PEState::FINISHED) {

        // —————————— 1) STEPPING ——————————
        // Cada hilo se suspende aquí hasta que System::step() incremente current_step_

        // 0) Esperamos a que current_step_ supere el último valor procesado
        {
            std::unique_lock<std::mutex> lk(step_mtx_);
            step_cv_.wait(lk, [&]{ return current_step_ > last_step; });
        }
        // Actualizamos el tracker local
        last_step = current_step_;

        // —————— 2) FINISHED POR PC FUERA DE RANGO ——————
        // Si el PC ya no apunta a ninguna instrucción válida, terminamos
        if (pe.get_pc() >= total_instr) {
            std::cout << "[PE " << pe_id 
                      << "] PC (" << pe.get_pc() 
                      << ") >= total_instr (" << total_instr 
                      << "), cambiando a FINISHED.\n";
            pe.set_state(PEState::FINISHED);
            break;
        }

        /* Cuando el PE este IDLE puede obtener una nueva instruccion */
        if (pe.get_state() == PEState::IDLE) {

            // —————— 3) FETCH: Obtenemos la instrucción actual ——————
            std::cout << "[PE " << pe.get_id() << "] State=IDLE. Getting new instruction...\n";

            // —————— 4) DECODE: La convertimos a Message ——————
            /* PE manda a convertir la instruccion del Instruction Memory,
               ubicada en el PC actual, a Message */
            Message actual_pe_message = pe.convert_to_message(pe.get_pc());

            /* PE dejará el Message recien creado como el actual a ejecutar
               luego de enviar a Interconnect */
            // 4) Almacenamos el mensaje en el PE (para debug o uso interno)
            pe.set_actual_message(actual_pe_message);

            // Cambiamos el estado a RUNNING, porque ya tenemos la petición lista
            pe.set_state(PEState::RUNNING);

            // Imprime ID del PE y el contenido formateado del mensaje
            /*std::cout << "  PE " << pe.get_id() << ": "
                    << actual_pe_message.to_string() << "\n";*/

            // —————— 5) Si es WRITE_MEM, leer cache ——————
            /* Si es WRITE_MEM se trae el dato de Cache */
            if (pe.get_actual_message().get_operation() == Operation::WRITE_MEM) {
                std::cout << "[PE " << pe.get_id() << "] WRITE_MEM detected – reading from cache:\n";

                // 2) Invocamos al método de LocalCache que simula la lectura
                /*cache.read_test(pe.get_actual_message().get_start_line(),
                                pe.get_actual_message().get_num_lines());*/

                // 2) Read the cache lines from disk
                uint32_t start = pe.get_actual_message().get_start_line();
                uint32_t count = pe.get_actual_message().get_num_lines();

                std::vector<std::vector<uint8_t>> blocks;
                try {
                    blocks = cache.read_cache_from_file(pe.get_id(), start, count);
                } catch (const std::exception& e) {
                    std::cerr << "[PE " << pe_id 
                            << "] Error reading cache lines: " << e.what() << "\n";
                }

                // 3) Stash the blocks into the Message payload
                pe.get_actual_message().set_data(blocks);

                // (Optional) debug print to verify
                std::cout << "[PE " << pe_id 
                        << "] Cached data attached to message (" 
                        << blocks.size() << " lines)\n";
            }

            // —————— 6) ISSUE: Enviamos el mensaje al Interconnect ——————
            std::cout << "[PE " << pe.get_id() << "] Sending message to Interconnect...\n";
            interconnect_->push_message(pe.get_actual_message());

            // 6) Cambiar el estado del PE a STALLED ya que se acaba de enviar la instruccion a ejecutar
            pe.set_state(PEState::STALLED);
            // 7) Cambiar el estado de respuesta del PE a WAITING ya que puede ahora esperar una respuesta
            pe.set_response_state(PEResponseState::WAITING);

            /* TODO: Revisar si hay alguna respuesta por venir */
            // —————— 7) (Pendiente) Esperar y procesar respuesta… ——————
            // Aquí podrías hacer:
            //   while (!interconnect_->has_response(pe_id)) std::this_thread::yield();
            //   Message resp = interconnect_->get_response(pe_id);
            //   pe.handle_response(resp);
            //   pe.set_response_state(PEResponseState::COMPLETED);

            // —————— 8) ADVANCE PC y volver a IDLE ——————
            pe.pc_plus_4();

            // TODO: Revisar esto porque puede quedar en Stalled si no se resuleve la respuesta
            pe.set_state(PEState::IDLE);

            // Debug: mostramos nuevo PC
            std::cout << "[PE " << pe_id 
                    << "] Avanzando PC a " << pe.get_pc() 
                    << ", estado IDLE\n";

        }

    }

    // Fin del thread
    std::cout << "\n[PE " << pe.get_id() << "] Thread ending...\n";
}

void System::join_pe_threads() {
    // Espera a cada hilo para evitar terminación prematura
    for (auto& t : pe_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    std::cout << "[System] All PE threads have joined.\n";
}

/* ------------------------------------ */
/*                                      */
/*         Interconnect's thread        */
/*                                      */
/* ------------------------------------ */

void System::start_interconnect_thread() {
    interconnect_thread_ = std::thread(&System::interconnect_execution_cycle, this);
    std::cout << "[System] Launched Interconnect thread\n";
}

void System::interconnect_execution_cycle() {
    std::cout << "[System] Interconnect Execution Cycle (thread) starting...\n";

    /* Tracker local de pasos */
    int last_step = 0;

    /* Ciclo de ejecucion correra hasta que el estado del PE llegue a FINISHED */
    while (interconnect_->get_state() != ICState::FINISHED) {

        // —————————— 1) STEPPING ——————————
        // Cada hilo se suspende aquí hasta que System::step() incremente current_step_

        // 0) Esperamos a que current_step_ supere el último valor procesado
        {
            std::unique_lock<std::mutex> lk(step_mtx_);
            step_cv_.wait(lk, [&]{ return current_step_ > last_step; });
        }
        // Actualizamos el tracker local
        last_step = current_step_;

        // ———————— 2) CHEQUEO DE FIN ————————
        // Si todos los PEs terminaron y NO hay mensajes en ninguna cola:
        // if (all_pes_finished() && interconnect_->all_queues_empty()) {
        
        /* TESTING: POR AHORA SOLO SE REVISA SI ESTÁ EMPTY IN_QUEUE (!!!)*/
        if (all_pes_finished() && interconnect_->all_queues_empty()) {
            std::cout << "[Interconnect] All work done, switching to FINISHED.\n";
            interconnect_->set_state(ICState::FINISHED);
            break;
        }

        // ———————— 3) IDLE vs PROCESSING ————————
        /* Si no hay mensajes en los queues pero los PEs no han terminado, stay IDLE*/
        if (interconnect_->all_queues_empty()) {
            // No hay peticiones: permanecemos IDLE
            interconnect_->set_state(ICState::IDLE);
            continue;  // esperamos el próximo step
        }

        // Hay mensajes: pasamos a PROCESSING
        interconnect_->set_state(ICState::PROCESSING);


        // ———————— 4) PROCESAR UNA PETICIÓN ————————

        // TODO: Revisar si hay mensajes en out_queue
        if (!interconnect_->out_queue_empty()) {

            /* Si hay Messages en out_queue, cada ciclo que pasa se sacara
               el primer Response hacia cada PE que haya, dependiendo del estado
               del PE. */

            // TESTING: Sacar Message de out_queue (por ahora se sacará el primero)
            Message response_to_PE = interconnect_->pop_out_queue_at(0);

        }       


        // TODO: Revisar si hay mensajes en mid_processing_queue
        if (!interconnect_->mid_processing_empty()) {
            /* Si hay Messages en mid_processing queue, cada ciclo que pasa
               se le tiene que ir restando un ciclo a cada latencia de cada uno*/

            /* Se tiene que revisar que si algun Message llega a 0 en el latency
               se tendría que pasar el out_queue para generar el response */

            // Se borraría el Message

            // Se crearía el response

            // Se pasaría el response al out_queue

            // 1) Averiguamos cuántos mensajes había al inicio de este paso
            size_t count = interconnect_->mid_processing_size();

            // 2) Procesamos cada uno **una sola vez**
            for (size_t i = 0; i < count; ++i) {
                // a) Sacamos el mensaje más antiguo (índice 0)
                Message msg_to_respond = interconnect_->pop_mid_processing_at(0);

                // b) Decrementamos su latencia en 1 ciclo
                msg_to_respond.decrement_latency();

                // c) Si ya completó la latencia, lo mandamos a out_queue_; 
                //    si no, lo volvemos a encolar en mid_processing para el próximo ciclo
                if (msg_to_respond.get_latency() == 0) {
                    interconnect_->push_out_queue(msg_to_respond);
                } else {
                    interconnect_->push_mid_processing(msg_to_respond);
                }
            }

        }

        
        if (!interconnect_->in_queue_empty()) {
            /* Si hay Messages en in_queue, cada ciclo se pasa la primera instruccion a mid_processing */
            /* Extrae el siguiente Message de in_queue para finalizar su espera por procesamiento */
            Message next_msg = interconnect_->pop_next();

            // 3) DECISION: ¿qué tipo de operación es?
            if (next_msg.get_operation() == Operation::READ_MEM) {
                // → Petición de lectura: iremos a memoria principal
                std::cout << "[IC] READ_MEM: preparando acceso a SharedMemory\n";

                // 1) Sacamos la dirección y el tamaño
                uint64_t address = next_msg.get_address();
                uint32_t size = next_msg.get_size();
                uint32_t   status    = 0x1;                  // OK por defecto

                // 3) Leemos del SharedMemory
                std::vector<std::vector<std::string>> memory_data;  // TODO: FIX!
                try {
                    memory_data = shared_memory_->read_shared_memory(address, size);
                } catch (const std::exception& e) {
                    std::cerr << "[IC] Error en READ_MEM: " << e.what() << "\n";
                    status = 0x0;
                    continue;
                }

                // 5) Creamos la respuesta WRITE_RESP con el estado de la operación
                Message read_resp(
                    Operation::READ_RESP,
                    /*src=*/-1,                     // Interconnect
                    /*dst=*/next_msg.get_src_id(),  // PE origen
                    /*addr=*/address,
                    /*qos=*/next_msg.get_qos(),
                    /*size=*/size,
                    /*num_lines=*/0,
                    /*start_line=*/0,
                    /*cache_line=*/0,
                    /*status=*/status,
                    /*data=*/{} // TODO: // TODO: FIX! ACTUALIZAR PAYLOAD LUEGO DE ARREGLAR FUNCIONES
                );

                // 6) Calculamos y asignamos la latencia
                // TODO: Calcular su latencia y asignarla (!)
                // TESTING ONLY
                read_resp.set_latency(12);

                // 7) Encolamos en la etapa media para simular la latencia
                interconnect_->push_mid_processing(read_resp);

            } else if (next_msg.get_operation() == Operation::WRITE_MEM) {
                // → Petición de escritura: datos vienen en next_msg.get_data()
                std::cout << "[IC] WRITE_MEM: preparando escritura en SharedMemory\n";

                // 1) Extraemos dirección y bloque de datos
                uint64_t   address   = next_msg.get_address();
                auto       blocks    = next_msg.get_data();  // vector<vector<uint8_t>>
                uint32_t   status    = 0x1;                  // OK por defecto

                try {
                    // 3) Volcamos al fichero de texto de SharedMemory
                    shared_memory_->write_shared_memory_lines(blocks, address);
                } catch (const std::exception& e) {
                    // 4) Si falla, lo reportamos y marcamos NOT_OK
                    std::cerr << "[IC] Error en WRITE_MEM: " << e.what() << "\n";
                    status = 0x0;
                    continue;
                }

                // 5) Creamos la respuesta WRITE_RESP con el estado de la operación
                Message write_resp(
                    Operation::WRITE_RESP,
                    /*src=*/-1,                     // Interconnect
                    /*dst=*/next_msg.get_src_id(),  // PE origen
                    /*addr=*/address,
                    /*qos=*/next_msg.get_qos(),
                    /*size=*/0,
                    /*num_lines=*/0,
                    /*start_line=*/0,
                    /*cache_line=*/0,
                    /*status=*/status,
                    /*data=*/{}                     // sin payload
                );

                // 6) TODO: Calcular su latencia y asignarla (!)
                // TESTING ONLY
                write_resp.set_latency(18);

                // 7) Encolamos la respuesta en out_queue_
                interconnect_->push_mid_processing(write_resp);

            } else if (next_msg.get_operation() == Operation::BROADCAST_INVALIDATE) {
                // → Broadcast: nada va a memoria, sino a invalidar caches
                std::cout << "[IC] BROADCAST_INVALIDATE: enviando INV_LINE a todos los PEs\n";
                // TODO: Generar tantos mensajes INV_LINE como PEs (excepto src)
                //       y encolarlos en out_queue_ directamente o en mid_pipeline según latencia

                // TODO: Calcular su latencia y asignarla (!)
                // TESTING ONLY
                next_msg.set_latency(8);

                // TODO: Ejecutar la instruccion

                /* Ingresa el Message en la cola de mid_processing para iniciar su ejecucion */
                interconnect_->push_mid_processing(next_msg);

            } else if (next_msg.get_operation() == Operation::INV_ACK) {
                // → Acknowledgment de invalidación: lleva de vuelta al originador
                std::cout << "[IC] INV_ACK: contabilizando ack para BROADCAST\n";
                // TODO: Llevar un contador interno; cuando llegue el último,
                //       encolas un INV_COMPLETE al PE origen

                // TODO: Calcular su latencia y asignarla (!)
                // TESTING ONLY
                next_msg.set_latency(4);

                // TODO: Ejecutar la instruccion

                /* Ingresa el Message en la cola de mid_processing para iniciar su ejecucion */
                interconnect_->push_mid_processing(next_msg);

            } else {
                // Cualquier otro caso (p.ej. END o UNDEFINED)
                std::cout << "[IC] Mensaje de tipo "
                  << static_cast<int>(next_msg.get_operation())
                  << " no procesado explícitamente\n";
            }
            
        }

        // TESTING: Imprime el in_queue
        std::cout << "\n[System Test] Dumping Interconnect in_queue:\n";
        interconnect_->debug_print_in_queue();

        // TESTING: Imprime el mid_processing_queue
        std::cout << "\n[System Test] Dumping Interconnect mid_processing_queue:\n";
        interconnect_->debug_print_mid_processing_queue();

        // TESTING: Imprime el out_queue
        std::cout << "\n[System Test] Dumping Interconnect out_queue:\n";
        interconnect_->debug_print_out_queue();


        // ———————— 5) VOLVER A IDLE ————————
        // Después de mover un mensaje, retomamos IDLE hasta el próximo step
        interconnect_->set_state(ICState::IDLE);
    }

    std::cout << "[System] Interconnect Execution Cycle (thread) ending...\n";
}

void System::join_interconnect_thread() {
    if (interconnect_thread_.joinable()) {
        interconnect_thread_.join();
    }
    std::cout << "[System] Interconnect thread has joined.\n";
}

/* --------------------------------------------------------------------------------------------- */

bool System::all_pes_finished() const {
    for (const auto& pe : pes_) {
        if (pe.get_state() != PEState::FINISHED) {
            return false;
        }
    }
    return true;
}

/* ---------------------------------------- Statistics ----------------------------------------- */

void System::report_statistics() const {
    std::cout << "\n[System] Reporting statistics for " << total_pes_
              << " PEs...\n";
    // TODO: Integrar con StatisticsUnit
    std::cout << "[System] (Placeholder)\n";
}

/* --------------------------------------------------------------------------------------------- */

/* ---------------------------------------- Testing -------------------------------------------- */

void System::debug_print() const {
    std::cout << "[System] Debug: General system state...";
    // Imprimir información de PEs
    std::cout << "\n[System] PEs and their QoS values:";
    for (const auto& pe : pes_) {
        std::cout << "  PE " << pe.get_id()
                  << " -> QoS=" << static_cast<int>(pe.get_qos()) << "";
    }
    // Llamar al debug del interconnect si existe
    if (interconnect_) {
        std::cout << "\n[System] Interconnect state:";
        interconnect_->debug_print();
    } else {
        std::cout << "\n[System] Interconnect not initialized.";
    }
}


/**
 * @brief Carga instrucciones binarias desde un archivo, las decodifica a objetos Message
 *        y los imprime en consola y en un archivo de salida.
 *
 * Esta función interpreta cada línea del archivo como una instrucción de 64 bits, 
 * donde el bit más significativo es el bit 63. Se ignoran los bits 63–43. A partir
 * de los bits restantes, se extraen los campos según el tipo de instrucción (WRITE_MEM,
 * READ_MEM, BROADCAST_INVALIDATE) y se construyen objetos Message. 
 * 
 * Cada mensaje decodificado se imprime en consola (en formato compacto y detallado)
 * y se guarda en el archivo mensajes_generados.txt.
 *
 * @param file_path Ruta al archivo que contiene instrucciones binarias en texto plano.
 * 
 * @note Se espera que cada línea del archivo contenga exactamente 64 caracteres ('0' o '1').
 * 
 * @warning Las líneas con menos de 64 bits serán ignoradas.
 */

void System::system_test_G(const std::string& file_path) {
	std::cout << "\n[TEST] Starting System Test G...\n";
    std::ifstream infile(file_path);
    std::ofstream outfile("mensajes_generados.txt");

    std::string line;
    int mensajes_generados = 0;

    if (!infile) {
        std::cerr << "[System] Error: could not open " << file_path << "\n";
        return;
    }

    outfile << "[System] Decoded messages from: " << file_path << "\n";

    while (std::getline(infile, line)) {
        if (line.length() < 64) {
            std::cerr << "[Warning] Ignoring line with length < 64: " << line << "\n";
            continue;
        }

        std::string opcode = line.substr(21, 2);  // Bits 42–41

        Message msg(Operation::UNDEFINED);

        if (opcode == "00") { // WRITE_MEM
            msg = Message(
                Operation::WRITE_MEM,
                std::bitset<5>(line.substr(23, 5)).to_ulong(),     // src: bits 40–36
                0,
                std::bitset<16>(line.substr(28, 16)).to_ulong(),   // address: bits 35–20
                std::bitset<4>(line.substr(60, 4)).to_ulong(),     // qos: bits 3–0
                0,
                std::bitset<8>(line.substr(44, 8)).to_ulong(),     // num_cache_lines: bits 19–12
                std::bitset<8>(line.substr(52, 8)).to_ulong(),     // start_cache_line: bits 11–4
                0, 0, {}
            );

        } else if (opcode == "01") { // READ_MEM
            msg = Message(
                Operation::READ_MEM,
                std::bitset<5>(line.substr(23, 5)).to_ulong(),
                0,
                std::bitset<16>(line.substr(28, 16)).to_ulong(),
                std::bitset<4>(line.substr(60, 4)).to_ulong(),
                std::bitset<8>(line.substr(44, 8)).to_ulong(),
                0, 0, 0, 0, {}
            );

        } else if (opcode == "10") { // BROADCAST_INVALIDATE
            msg = Message(
                Operation::BROADCAST_INVALIDATE,
                std::bitset<5>(line.substr(23, 5)).to_ulong(),
                0,
                0,
                std::bitset<4>(line.substr(60, 4)).to_ulong(),
                0, 0, 0,
                std::bitset<8>(line.substr(36, 8)).to_ulong(),     // cache_lines (bits 27–20)
                0, {}
            );

        } else {
            std::cerr << "[Warning] Opcode no reconocido: " << opcode << "\n";
            continue;
        }

        // Imprimir en consola y escribir en archivo
        std::cout << "[DEBUG] Mensaje creado: " << msg.to_string() << std::endl;
        outfile << msg.to_string() << "\n";
        mensajes_generados++;
    }

    outfile << "[System] Message decoding complete.\n";
    outfile << "[System] Total messages decoded: " << mensajes_generados << "\n";
    std::cout << "[System] Total messages decoded: " << mensajes_generados << "\n";

    outfile.close();
}
    

void System::system_test_R() {
	std::cout << "\n[TEST] Starting System Test R...\n";

    std::cout << "\n[TEST] System Test Complete.\n";
}

/* --------------------------------------------------------------------------------------------- */
