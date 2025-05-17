#include "../include/System.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <thread>
#include <bitset>
#include <cmath>

/* ---------------------------------------- Constructor ---------------------------------------- */

System::System(int num_pes, ArbitScheme scheme, bool stepping_enabled)
    : total_pes_(num_pes), scheme_(scheme), stepping_enabled_(stepping_enabled){
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

void System::set_stepping_enabled(bool enable) {
    stepping_enabled_ = enable;
}

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

    // 3) Bucle principal: stepping o auto-run
    if (stepping_enabled_) {
        std::string line;
        while (!all_pes_finished() || interconnect_->get_state() != ICState::FINISHED) {
            std::cout << "\nPRESS [Enter] TO ADVANCE ONE CYCLE…\n";
            std::getline(std::cin, line);
            step();
        }
    } else {
        // Auto-run: ejecuta step() en bucle hasta terminar
        while (!all_pes_finished() || interconnect_->get_state() != ICState::FINISHED) {
            step();
        }
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

    /* Ciclo de ejecucion correra hasta que el estado del PE llegue a FINISHED */
    while (pe.get_state() != PEState::FINISHED) {

        // —————————— 1) STEPPING ——————————
        // Cada hilo se suspende aquí hasta que System::step() incremente current_step_

        // Esperamos a que current_step_ supere el último valor procesado
        {
            std::unique_lock<std::mutex> lk(step_mtx_);
            step_cv_.wait(lk, [&]{ return current_step_ > last_step; });
        }
        // Actualizamos el tracker local
        last_step = current_step_;

        // —— 2) CHEQUEO DE RESPUESTA —— 
        if (pe.get_response_state() == PEResponseState::WAITING) {

            /* Revisa si hay un response en el out_queue */
            if (interconnect_->has_response(pe_id)) {

                // Se pondra a procesar la respuesta
                pe.set_response_state(PEResponseState::PROCESSING);

                // 1) Sacamos UNA respuesta para este PE
                Message resp = interconnect_->pop_response(pe_id);

                // 6) Calculamos y asignamos la latencia
                resp.increment_full_latency(10);
                /*********** Debugging prints ***********/
                /**/ resp.print_latency_debug();      /**/
                /**/ resp.print_full_latency_debug(); /**/
                /****************************************/

                // 2) El PE la procesa
                std::cout << "[PE " << pe_id << "] Received response: "
                        << resp.to_string() << "\n";

                // 1) CASO INV_LINE
                if (resp.get_operation() == Operation::INV_LINE) {
                    // 1.a) Invalida la línea en el cache local
                    cache.invalidate_line(resp.get_cache_line(), pe_id);
                    
                    // 1.b) Construye el ACK usando el mismo broadcast_id
                    Message inv_ack(
                        Operation::INV_ACK,
                        /*src=*/pe_id,
                        /*dst=*/-1,  // Interconnect (no lo usas en tu diseño)
                        /*addr=*/0,
                        /*qos=*/resp.get_qos(), // Mantiene el QoS del PE que envio el B_I
                        /*size=*/0,
                        /*num_lines=*/0,
                        /*start_line=*/0,
                        /*cache_line=*/0,
                        /*status=*/0,
                        /*data=*/{}
                    );

                    inv_ack.set_broadcast_id(resp.get_broadcast_id());

                    // 6) Calculamos y asignamos la latencia
                    resp.increment_full_latency(6 + 3 + 4);

                    /*********** Debugging prints ***********/
                    /**/ resp.print_latency_debug();      /**/
                    /**/ resp.print_full_latency_debug(); /**/
                    /****************************************/

                    /*TODO: FIN DE MESSAGE PATH -> EXPORTAR DATOS DE LATENCIA*/
                    log_message_metrics(resp);

                    // Se envia al in_queue del Interconnect como un mensaje asincrono
                    interconnect_->push_message(inv_ack);

                    std::cout << "[PE " << pe_id 
                            << "] Procesado INV_LINE (línea " << resp.get_cache_line()
                            << "), enviado INV_ACK con bid=" << resp.get_broadcast_id() << "\n";

                    /* State check */
                    if(pe.get_actual_message().get_operation() == Operation::BROADCAST_INVALIDATE) {
                        pe.set_response_state(PEResponseState::WAITING);
                    } else {
                        /* Check PE states */
                        // Reemplazo
                        bool more_out = interconnect_->has_pending_responses(pe_id);
                        bool more_mid = interconnect_->has_pending_mid_processing(pe_id);

                        if (more_out || more_mid) {
                            // Aún hay respuestas para ti, sigues “atrapado”
                            pe.set_state(PEState::STALLED);
                            pe.set_response_state(PEResponseState::WAITING);
                        } else {
                            // Ya no queda ninguna respuesta pendiente
                            pe.set_response_state(PEResponseState::COMPLETED);
                            pe.set_state(PEState::IDLE);
                        }
                    }

                }

                // 2) CASO READ_RESP
                else if (resp.get_operation() == Operation::READ_RESP) {
                    // Por ahora solo imprimimos información básica
                    std::cout << "[PE " << pe_id << "] READ_RESP recibido:\n"
                            << "    Dirección solicitada: 0x" << std::hex << resp.get_address() << std::dec << "\n"
                            << "    Tamaño solicitado: " << resp.get_size() << " bytes\n"
                            << "    Líneas de cache leídas: " << resp.get_num_lines() << "\n"
                            << "    Payload (líneas): " << resp.get_data().size() << "\n";
                    // Opcional: imprimir primer byte de cada línea
                    for (size_t i = 0; i < resp.get_data().size(); ++i) {
                        const auto& line = resp.get_data()[i];
                        if (!line.empty()) {
                            std::cout << "      Línea[" << i << "][0] = 0x"
                                    << std::hex << static_cast<int>(line[0]) << std::dec << "\n";
                        }
                    }

                    // Escribe en cache quemado en 0 lol, sorry profe
                    cache.write_cache_lines(pe_id, resp.get_start_line(), resp.get_data());

                    // Calculamos y asignamos la latencia
                    resp.increment_full_latency(4 * resp.get_size());
                    /*********** Debugging prints ***********/
                    /**/ resp.print_latency_debug();      /**/
                    /**/ resp.print_full_latency_debug(); /**/
                    /****************************************/

                    /*TODO: FIN DE MESSAGE PATH -> EXPORTAR DATOS DE LATENCIA*/
                    log_message_metrics(resp);

                    /* Check PE states */
                    // Reemplazo
                    bool more_out = interconnect_->has_pending_responses(pe_id);
                    bool more_mid = interconnect_->has_pending_mid_processing(pe_id);

                    if (more_out || more_mid) {
                        // Aún hay respuestas para ti, sigues “atrapado”
                        pe.set_state(PEState::STALLED);
                        pe.set_response_state(PEResponseState::WAITING);
                    } else {
                        // Ya no queda ninguna respuesta pendiente
                        pe.set_response_state(PEResponseState::COMPLETED);
                        pe.set_state(PEState::IDLE);
                    }

                // 3) CASO WRITE_RESP
                } else if (resp.get_operation() == Operation::WRITE_RESP) {
                    std::cout << "[PE " << pe_id << "] WRITE_RESP recibido:\n"
                            << "    Dirección escrita: 0x" << std::hex << resp.get_address() << std::dec << "\n"
                            << "    Estado (status): 0x" << std::hex << resp.get_status() << std::dec << "\n";

                    // Calculamos y asignamos la latencia
                    resp.increment_full_latency(5);
                    /*********** Debugging prints ***********/
                    /**/ resp.print_latency_debug();      /**/
                    /**/ resp.print_full_latency_debug(); /**/
                    /****************************************/

                    /*TODO: FIN DE MESSAGE PATH -> EXPORTAR DATOS DE LATENCIA*/
                    log_message_metrics(resp);

                    /* Check PE states */
                    // Reemplazo
                    bool more_out = interconnect_->has_pending_responses(pe_id);
                    bool more_mid = interconnect_->has_pending_mid_processing(pe_id);

                    if (more_out || more_mid) {
                        // Aún hay respuestas para ti, sigues “atrapado”
                        pe.set_state(PEState::STALLED);
                        pe.set_response_state(PEResponseState::WAITING);
                    } else {
                        // Ya no queda ninguna respuesta pendiente
                        pe.set_response_state(PEResponseState::COMPLETED);
                        pe.set_state(PEState::IDLE);
                    }

                // 4) CASO INV_COMPLETE
                } else if (resp.get_operation() == Operation::INV_COMPLETE) {
                    std::cout << "[PE " << pe_id << "] INV_COMPLETE recibido:\n"
                            << "    Broadcast ID: " << resp.get_broadcast_id() << "\n"
                            << "    Línea inválidada confirmada por todos los PEs.\n";

                    // Calculamos y asignamos la latencia
                    resp.increment_full_latency(5);
                    /*********** Debugging prints ***********/
                    /**/ resp.print_latency_debug();      /**/
                    /**/ resp.print_full_latency_debug(); /**/
                    /****************************************/

                    /*TODO: FIN DE MESSAGE PATH -> EXPORTAR DATOS DE LATENCIA*/
                    log_message_metrics(resp);

                    /* Check PE states */
                    // Reemplazo
                    bool more_out = interconnect_->has_pending_responses(pe_id);
                    bool more_mid = interconnect_->has_pending_mid_processing(pe_id);

                    if (more_out || more_mid) {
                        // Aún hay respuestas para ti, sigues “atrapado”
                        pe.set_state(PEState::STALLED);
                        pe.set_response_state(PEResponseState::WAITING);
                    } else {
                        // Ya no queda ninguna respuesta pendiente
                        pe.set_response_state(PEResponseState::COMPLETED);
                        pe.set_state(PEState::IDLE);
                    }
                }

            } else {
                std::cout << "[PE " << pe_id << "] Waiting for response (?)"
                          << " - PC: " << pe.get_pc() << "\n";

                /* Still on WAITING */
            }
            
        }

        /* ————— 4) Cuando el PE este IDLE puede obtener una nueva instruccion */
        if (pe.get_state() == PEState::IDLE && pe.get_pc() < total_instr) {

            // —————— 4) FETCH: Obtenemos la instrucción actual ——————
            std::cout << "[PE " << pe.get_id() << "] State=IDLE. Getting new instruction...\n";

            // —————— 5) DECODE: La convertimos a Message ——————
            /* PE manda a convertir la instruccion del Instruction Memory,
               ubicada en el PC actual, a Message */
            Message actual_pe_message = pe.convert_to_message(pe.get_pc());

            /* PE dejará el Message recien creado como el actual a ejecutar
               luego de enviar a Interconnect */
            // 4) Almacenamos el mensaje en el PE (para debug o uso interno)
            pe.set_actual_message(actual_pe_message);

            /* Incremento de latencia: Fetch Instr*/
            pe.get_actual_message().set_full_latency(3); // Ya que sera la primera vez que se agrega
            /******************** Debugging prints *********************/
            /**/ pe.get_actual_message().print_latency_debug();      /**/
            /**/ pe.get_actual_message().print_full_latency_debug(); /**/
            /***********************************************************/
            
            // Cambiamos el estado a RUNNING, porque ya tenemos la petición lista
            pe.set_state(PEState::RUNNING);

            // Imprime ID del PE y el contenido formateado del mensaje
            /*std::cout << "  PE " << pe.get_id() << ": "
                    << actual_pe_message.to_string() << "\n";*/

            // —————— 6) Si es WRITE_MEM, leer cache ——————
            /* Si es WRITE_MEM se trae el dato de Cache */
            if (pe.get_actual_message().get_operation() == Operation::WRITE_MEM) {
                std::cout << "[PE " << pe.get_id() << "] WRITE_MEM detected – reading from cache:\n";

                // 1) Invocamos al método de LocalCache que simula la lectura
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

                /* Incremento de latencia: Cache Read */
                pe.get_actual_message().increment_full_latency(4 * count);
                /******************** Debugging prints *********************/
                /**/ pe.get_actual_message().print_latency_debug();      /**/
                /**/ pe.get_actual_message().print_full_latency_debug(); /**/
                /***********************************************************/

                // (Optional) debug print to verify
                std::cout << "[PE " << pe_id 
                        << "] Cached data attached to message (" 
                        << blocks.size() << " lines)\n";
            }

            // —————— 7) ISSUE: Enviamos el mensaje al Interconnect ——————
            std::cout << "[PE " << pe.get_id() << "] Sending message to Interconnect...\n";

            /* Incremento de latencia: Send Inter */
            pe.get_actual_message().increment_full_latency(5);
            /******************** Debugging prints *********************/
            /**/ pe.get_actual_message().print_latency_debug();      /**/
            /**/ pe.get_actual_message().print_full_latency_debug(); /**/
            /***********************************************************/

            interconnect_->push_message(pe.get_actual_message());

            // 6) Cambiar el estado del PE a STALLED ya que se acaba de enviar la instruccion a ejecutar
            pe.set_state(PEState::STALLED);
            // 7) Cambiar el estado de respuesta del PE a WAITING ya que puede ahora esperar una respuesta
            pe.set_response_state(PEResponseState::WAITING);

            // —————— 8) ADVANCE PC y volver a IDLE ——————
            pe.pc_plus_4();

            // TODO: Revisar esto porque puede quedar en Stalled si no se resuleve la respuesta
            //pe.set_state(PEState::IDLE);

            // TODO: Revisar si dejar aqui el cambio de nuevo al estado de WAINTING para una respuesta
            //pe.set_response_state(PEResponseState::WAITING);

            // Debug: mostramos nuevo PC
            std::cout << "[PE " << pe_id 
                    << "] Avanzando PC a " << pe.get_pc() 
                    << ", estado " << pe.state_to_string() << ".\n";

        }
        /* Cuando el PE este IDLE puede obtener una nueva instruccion */
        else if (pe.get_state() == PEState::STALLED) {


            // El estado de respuesta del PE seria WAITING ya que esta esperando una respuesta
            pe.set_response_state(PEResponseState::WAITING);
            std::cout << "[PE " << pe_id << "] state = STALLED. Awaiting response.\n";

        } else if (pe.get_response_state() == PEResponseState::WAITING) {

            std::cout << "[PE " << pe_id 
                    << "]  response state: " << pe.state_to_string() << " (?).\n";

        } else {
            pe.set_response_state(PEResponseState::COMPLETED);
            // Debug: mostramos nuevo PC
            std::cout << "[PE " << pe_id 
                    << "]  estado " << pe.state_to_string() << " (?).\n";
        }


        /* CHANGE */
        /* CAMBIO DE POSICION DONDE SE REVISA ESTO */
        // ————— 3) FINISHED POR PC FUERA DE RANGO ——————
        // Si el PC ya no apunta a ninguna instrucción válida, terminamos
        if (pe.get_pc() >= total_instr &&
            pe.get_state() != PEState::STALLED &&
            !interconnect_->has_pending_responses(pe_id) &&
            !interconnect_->has_pending_mid_processing(pe_id) &&
            pe.get_response_state() != PEResponseState::WAITING) {
                std::cout << "[PE " << pe_id 
                        << "] PC (" << pe.get_pc() 
                        << ") > total_instr (" << total_instr 
                        << ") & all queues are empty "
                        << "& not waiting for response. Cambiando a FINISHED.\n";
                pe.set_state(PEState::FINISHED);
                break;
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
        /* Condicion de parada */
        // Si todos los PEs terminaron y NO hay mensajes en ninguna cola:        
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
        // TODO: Revisar si esto es necesario ademas de la logica que hay en el thread de PE
        //if (!interconnect_->out_queue_empty()) {

            /* TODO: Si hay Messages en out_queue, cada ciclo que pasa se sacara
               el primer Response hacia CADA PE que haya, dependiendo del estado
               del PE. */

            // TESTING: Sacar Message de out_queue (por ahora se sacará el primero)
            //Message response_to_PE = interconnect_->pop_out_queue_at(0);
        //}       


        // TODO: Revisar si hay mensajes en mid_processing_queue
        if (!interconnect_->mid_processing_empty()) {

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

            double latency_increment;

            /* Incremento de latencia: Wait Queue*/
            if (scheme_ == ArbitScheme::PRIORITY) {
                latency_increment = 2/*0*/ * 
                                    (next_msg.get_num_lines() + next_msg.get_size() + 1) *
                                    (1/next_msg.get_qos());
            } else {
                latency_increment = 2/*0*/ *
                                    (next_msg.get_num_lines() + next_msg.get_size() + 1);
            }

            std::cerr << "[Debug] latency_increment = " << latency_increment << ".\n";

            next_msg.increment_full_latency(static_cast<uint32_t>(std::round(latency_increment)));
            next_msg.set_latency(static_cast<uint32_t>(std::round(latency_increment)));

            /************* Debugging prints *************/
            /**/ next_msg.print_latency_debug();      /**/
            /**/ next_msg.print_full_latency_debug(); /**/
            /********************************************/
            

            // 3) DECISION: ¿qué tipo de operación es?
            if (next_msg.get_operation() == Operation::READ_MEM) {
                // → Petición de lectura: iremos a memoria principal
                std::cout << "[IC] READ_MEM: preparando acceso a SharedMemory\n";

                // 1) Sacamos la dirección y el tamaño
                uint64_t address = next_msg.get_address();
                uint32_t size = next_msg.get_size();
                uint32_t   status    = 0x1;                  // OK por defecto

                std::cerr << "[IC] READ_MEM escribirá " << size << " bytes.\n";

                // 3) Leemos del SharedMemory
                std::vector<std::vector<std::uint8_t>> memory_data;
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
                    /*data=*/memory_data
                );

                // Pasar latencia del Message de Instruccion al de Respuesta
                read_resp.set_full_latency(next_msg.get_full_latency()); /* CHANGE */
                read_resp.set_latency(next_msg.get_latency()); /* CHANGE */

                /************* Debugging prints **************/
                /**/ read_resp.print_latency_debug();      /**/
                /**/ read_resp.print_full_latency_debug(); /**/
                /*********************************************/

                // 6) Calculamos y asignamos la latencia
                uint32_t incr_lat = (60 + size); /* CHANGE */
                read_resp.increment_full_latency(incr_lat);
                read_resp.increment_latency(incr_lat);

                /************* Debugging prints **************/
                /**/ read_resp.print_latency_debug();      /**/
                /**/ read_resp.print_full_latency_debug(); /**/
                /*********************************************/

                // 7) Encolamos en la etapa media para simular la latencia
                interconnect_->push_mid_processing(read_resp);

            } else if (next_msg.get_operation() == Operation::WRITE_MEM) {
                // → Petición de escritura: datos vienen en next_msg.get_data()
                std::cout << "[IC] WRITE_MEM: preparando escritura en SharedMemory\n";

                // 1) Extraemos dirección y bloque de datos
                uint32_t   num_lines = next_msg.get_num_lines(); 
                uint64_t   address   = next_msg.get_address();
                auto       blocks    = next_msg.get_data();  // vector<vector<uint8_t>>
                uint32_t   status    = 0x1;                  // OK por defecto

                std::cerr << "[IC] WRITE_MEM escribirá " << num_lines << " lineas.\n";

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
                    /*num_lines=*/next_msg.get_num_lines(),
                    /*start_line=*/0,
                    /*cache_line=*/0,
                    /*status=*/status,
                    /*data=*/{}                     // sin payload
                );

                // Pasar latencia del Message de Instruccion al de Respuesta
                write_resp.set_full_latency(next_msg.get_full_latency());  /* CHANGE */
                write_resp.set_latency(next_msg.get_latency());  /* CHANGE */

                /************** Debugging prints **************/
                /**/ write_resp.print_latency_debug();      /**/
                /**/ write_resp.print_full_latency_debug(); /**/
                /**********************************************/

                // 6) Calculamos y asignamos la latencia
                uint32_t incr_lat = (80 + num_lines) * (num_lines * 0.04); /* CHANGE */
                write_resp.increment_full_latency(incr_lat);
                write_resp.increment_latency(incr_lat);

                /************** Debugging prints **************/
                /**/ write_resp.print_latency_debug();      /**/
                /**/ write_resp.print_full_latency_debug(); /**/
                /**********************************************/

                // 7) Encolamos en la etapa media para simular la latencia
                interconnect_->push_mid_processing(write_resp);

            } else if (next_msg.get_operation() == Operation::BROADCAST_INVALIDATE) {
                // → Broadcast: invalidar cache line en todos los PEs
                uint32_t src_pe     = next_msg.get_src_id();
                uint32_t qos        = next_msg.get_qos();
                uint32_t cache_line = next_msg.get_cache_line();

                std::cout << "[IC] BROADCAST_INVALIDATE: enviando INV_LINE a todos los PEs (incluyendo src=" 
                        << src_pe << ")\n";

                /* Se obtiene un nuevo ID para este nuevo BROADCAST */
                uint32_t bid = interconnect_->register_broadcast(src_pe);

                // Para cada PE creamos un INV_LINE
                for (int pid = 0; pid < total_pes_; ++pid) {
                    // 1) Construir el mensaje de invalidación de línea
                    Message inv_line_msg(
                        Operation::INV_LINE,  // operación
                        /* src */ src_pe,     // PE origen del broadcast
                        /* dst */ pid,        // destino: cada PE
                        /* addr */ 0,         // no usamos ADDR aquí
                        /* qos */ qos,        // heredamos el QoS original
                        /* size */ 0,         // no aplica
                        /* num_lines */ 0, 
                        /* start_line */ 0,
                        /* cache_line */ cache_line,
                        /* status */ 0,
                        /* data */ {}         // sin payload
                    );

                    // Pasar latencia del Message de Instruccion al de Respuesta
                    inv_line_msg.set_full_latency(next_msg.get_full_latency());
                    inv_line_msg.set_latency(next_msg.get_latency());

                    /*************** Debugging prints ***************/
                    /**/ inv_line_msg.print_latency_debug();      /**/
                    /**/ inv_line_msg.print_full_latency_debug(); /**/
                    /************************************************/

                    // 2) Se clava el broadcast_id en el Message para que se propague
                    inv_line_msg.set_broadcast_id(bid);

                    // 6) Calculamos y asignamos la latencia
                    uint32_t incr_lat = 6;
                    inv_line_msg.increment_full_latency(incr_lat);
                    inv_line_msg.increment_latency(incr_lat);

                    /*************** Debugging prints ***************/
                    /**/ inv_line_msg.print_latency_debug();      /**/
                    /**/ inv_line_msg.print_full_latency_debug(); /**/
                    /************************************************/

                    // 3) Encolamos en la etapa media para simular la latencia
                    interconnect_->push_mid_processing(inv_line_msg);
                }

            } else if (next_msg.get_operation() == Operation::INV_ACK) {
                // → Acknowledgment de invalidación: contabilizando ack para el broadcast
                std::cout << "[IC] INV_ACK: contabilizando ack para BROADCAST\n";

                uint32_t bid    = next_msg.get_broadcast_id(); // identificador del broadcast
                uint32_t qos    = next_msg.get_qos();
                int      origin = -1;
                bool     complete = false;

                {
                    // 1) Protegemos el acceso al mapa de broadcasts pendientes
                    std::lock_guard<std::mutex> lk(interconnect_->broadcast_mtx_);
                    auto it = interconnect_->pending_broadcasts_.find(bid);

                    /* Si no esta fuera de rango */
                    if (it != interconnect_->pending_broadcasts_.end()) {
                        // 2) Restamos un ACK pendiente
                        /* it->second da acceso al valor de PendingBroadcast, first seria su key*/
                        it->second.pending_acks--;   // restar un ACK pendiente
                        origin = it->second.origin_pe;  // leer quién inició el broadcast
                        std::cout << "[IC] INV_ACK recibido para broadcast " << bid
                                << ", faltan " << it->second.pending_acks << " ACKs\n";

                        // 3) Si ya no falta ninguno, marcamos completo y borramos el registro
                        if (it->second.pending_acks == 0) {
                            complete = true;
                            interconnect_->pending_broadcasts_.erase(it);
                        }
                    } else {
                        std::cerr << "[IC] INV_ACK con broadcast_id inválido: " << bid << "\n";
                    }
                }

                // TODO: Como medir esta latencia?
                // 4) Asignamos latencia de ACK (por ejemplo, 1 ciclo)
                // next_msg.set_latency(1);
                // 5) Lo metemos en mid_pipeline para procesar ese ACK
                // interconnect_->push_mid_processing(next_msg);

                // 6) Si este ACK cierra el broadcast, generamos INV_COMPLETE
                if (complete && origin >= 0) {
                    Message inv_complete(
                        Operation::INV_COMPLETE,
                        /*src=*/-1,        // Interconnect
                        /*dst=*/origin,    // PE que inició el broadcast
                        /*addr=*/0,
                        /*qos=*/qos,
                        /* size */ 0,         // no aplica
                        /* num_lines */ 0, 
                        /* start_line */ 0,
                        /*cache_line=*/0,
                        /*status=*/0,
                        /*data=*/{}
                    );

                    // Pasar latencia del Message de Instruccion al de Respuesta
                    inv_complete.set_full_latency(5 * total_pes_);
                    inv_complete.set_latency(5 * total_pes_);

                    /*************** Debugging prints ***************/
                    /**/ inv_complete.print_latency_debug();      /**/
                    /**/ inv_complete.print_full_latency_debug(); /**/
                    /************************************************/

                    inv_complete.set_broadcast_id(bid);

                    // 6) Calculamos y asignamos la latencia
                    uint32_t incr_lat = 5;
                    inv_complete.increment_full_latency(incr_lat);
                    inv_complete.increment_latency(incr_lat);

                    /*************** Debugging prints ***************/
                    /**/ inv_complete.print_latency_debug();      /**/
                    /**/ inv_complete.print_full_latency_debug(); /**/
                    /************************************************/

                    // 8) Encolamos en la etapa media para simular la latencia
                    interconnect_->push_mid_processing(inv_complete);

                    std::cout << "[IC] Todos los INV_ACK de broadcast " << bid
                            << " recibidos: encolando INV_COMPLETE para PE " << origin << "\n";
                }

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


        // ———————— 5) VOLVER A IDLE o marcar FINISHED ————————
        if (all_pes_finished() && interconnect_->all_queues_empty()) {
            // Si todos los PEs ya completaron y no queda nada en ninguna cola:
            interconnect_->set_state(ICState::FINISHED);
        } else {
            // En cualquier otro caso, sólo volvemos a IDLE si no hay mensajes pendientes
            // (mid + out)
            if (interconnect_->all_queues_empty()) {
                interconnect_->set_state(ICState::IDLE);
            }
            // si todavía quedan mensajes, mantenemos el estado en PROCESSING
        }
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

// Helper interno para convertir la operación a texto
const char* System::operation_to_string(Operation op) {
    switch (op) {
        case Operation::READ_MEM:             return "READ_MEM";
        case Operation::WRITE_MEM:            return "WRITE_MEM";
        case Operation::BROADCAST_INVALIDATE: return "BROADCAST_INVALIDATE";
        case Operation::INV_LINE:             return "INV_LINE";
        case Operation::INV_ACK:              return "INV_ACK";
        case Operation::INV_COMPLETE:         return "INV_COMPLETE";
        case Operation::READ_RESP:            return "READ_RESP";
        case Operation::WRITE_RESP:           return "WRITE_RESP";
        case Operation::END:                  return "END";
        default:                              return "UNDEFINED";
    }
}

void System::log_message_metrics(const Message& msg,
                                 const std::string& filename) const {
    // Abrir en modo append
    std::ofstream out(filename, std::ios::app);
    if (!out.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo de log: " + filename);
    }

    // Recopilar datos
    int        pe_id         = msg.get_dest_id();
    uint8_t    qos           = msg.get_qos();
    const char* op_str       = operation_to_string(msg.get_operation());
    uint32_t   size_bytes    = msg.get_size() * 4;
    uint32_t   num_bytes     = msg.get_num_lines() * 16;
    uint32_t   latency       = msg.get_full_latency();
    // uint32_t   bandwidth     = msg.get_bandwidth();

    // Escribir línea de log
    out << " " << pe_id << "  "
        << " 0x" << std::hex << static_cast<int>(qos) << std::dec << "  "
        << " " << op_str << "  "
        << " " << size_bytes << "  "
        << " " << num_bytes << "  "
        << " " << latency << " \n";
        // << "[" << bandwidth << "]\n";

    out.close();
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


void System::system_test_G(const std::string& file_path) {
	std::cout << "\n[TEST] Starting System Test G...\n";

    std::cout << "\n[TEST] System Test Complete.\n";
}

void System::system_test_R() {
	std::cout << "\n[TEST] Starting System Test R...\n";

    std::cout << "\n[TEST] System Test Complete.\n";
}

/* --------------------------------------------------------------------------------------------- */
