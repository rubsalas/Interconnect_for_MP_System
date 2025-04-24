#include "../include/Message.h"
#include <cstdio>

Message::Message(Operation operation,
                 int src,
                 int dst,
                 uint64_t addr,
                 uint8_t qos,
                 uint32_t size,
                 uint32_t num_lines,
                 uint32_t start_line,
                 uint32_t cache_line,
                 uint32_t status,
                 std::vector<uint32_t> data)
    : operation_(operation), src_id_(src), dest_id_(dst), address_(addr), qos_(qos),
      size_(size), num_lines_(num_lines), start_line_(start_line),
      cache_line_(cache_line), status_(status), data_(std::move(data)) {}

Operation Message::operation() const { return operation_; }
int Message::src_id() const { return src_id_; }
int Message::dest_id() const { return dest_id_; }
uint64_t Message::address() const { return address_; }
uint8_t Message::qos() const { return qos_; }
uint32_t Message::size() const { return size_; }
uint32_t Message::num_lines() const { return num_lines_; }
uint32_t Message::start_line() const { return start_line_; }
uint32_t Message::cache_line() const { return cache_line_; }
uint32_t Message::status() const { return status_; }
const std::vector<uint32_t>& Message::data() const { return data_; }

std::string Message::to_string() const {
    auto operation_name = [](Operation k) {
        switch (k) {
            case Operation::READ_MEM:             return "READ_MEM";
            case Operation::WRITE_MEM:            return "WRITE_MEM";
            case Operation::BROADCAST_INVALIDATE: return "BCAST_INV";
            case Operation::INV_ACK:              return "INV_ACK";
            case Operation::INV_COMPLETE:         return "INV_COMPLETE";
            case Operation::READ_RESP:            return "READ_RESP";
            case Operation::WRITE_RESP:           return "WRITE_RESP";
            case Operation::UNDEFINED:            return "UNDEFINED";
        }
        return "UNKNOWN";
    };

    char buf[256];
    std::snprintf(buf, sizeof(buf),
                  "[MSG %s src=%d dst=%d qos=%u addr=0x%016llX size=%u nl=%u sl=%u cl=%u status=%u data_words=%zu]",
                  operation_name(operation_), src_id_, dest_id_, qos_,
                  static_cast<unsigned long long>(address_), size_, num_lines_,
                  start_line_, cache_line_, status_, data_.size());
    return std::string(buf);
}
