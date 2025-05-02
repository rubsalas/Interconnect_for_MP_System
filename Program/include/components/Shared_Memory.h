#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>

class SharedMemory {
private:
    static constexpr size_t MEMORY_SIZE = 4096;
    std::vector<uint32_t> data;

public:
    SharedMemory();

    uint32_t load(size_t address) const;
    void store(size_t address, uint32_t value);
    size_t size() const;

    void fill_random();
    void dump_to_binary_file(const std::string& output_filename) const;
    void dump_to_text_file(const std::string& output_filename) const; // <<--- Nueva función
};

#endif // SHARED_MEMORY_H
