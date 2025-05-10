#include "../Program/include/components/Local_Cache.h"
#include <iostream>
#include <vector>

/*
 * Compile:
 * g++ -std=c++20 -Wall ../Program/src/components/Local_Cache.cpp cache_main.cpp -o test_caches
 * ./test_caches <cant. PEs> 
 */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <cantidad_de_caches>\n";
        return 1;
    }

    int num_caches = std::stoi(argv[1]);
    if (num_caches <= 0) {
        std::cerr << "Error: La cantidad de caches debe ser mayor que 0.\n";
        return 1;
    }

    std::vector<LocalCache> caches(num_caches);

    for (int i = 0; i < num_caches; ++i) {
        caches[i].fill_random();
        //std::string filename = "caches/cache_" + std::to_string(i) + ".txt"; // <<--- Ya no se usa
        try {
            caches[i].dump_to_text_file();
            std::cout << "Cache " << i << " exportada\n";
        } catch (const std::exception& e) {
            std::cerr << "Error al exportar cache " << i << ": " << e.what() << "\n";
        }
    }

    return 0;
}
