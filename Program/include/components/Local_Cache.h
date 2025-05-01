#ifndef LOCAL_CACHE_H
#define LOCAL_CACHE_H

#include <vector>
#include <array>
#include <cstdint>
#include <string>

class LocalCache {
private:
    static constexpr size_t BLOCKS = 128;
    static constexpr size_t BLOCK_SIZE = 16; // 16 bytes
    std::vector<std::array<uint8_t, BLOCK_SIZE>> cache_data;

public:
    LocalCache();
    
    void fill_random();
    void dump_to_text_file(const std::string& output_filename) const;
};

#endif // LOCAL_CACHE_H
