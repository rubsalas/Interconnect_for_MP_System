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

/* ----------------------------------- Getters & Setters --------------------------------------- */

Operation Message::get_operation() const { return operation_; }
int Message::get_src_id() const { return src_id_; }
int Message::get_dest_id() const { return dest_id_; }
uint64_t Message::get_address() const { return address_; }
uint8_t Message::get_qos() const { return qos_; }
uint32_t Message::get_size() const { return size_; }
uint32_t Message::get_num_lines() const { return num_lines_; }
uint32_t Message::get_start_line() const { return start_line_; }
uint32_t Message::get_cache_line() const { return cache_line_; }
uint32_t Message::get_status() const { return status_; }
const std::vector<uint32_t>& Message::get_data() const { return data_; }

void Message::set_operation(Operation op) { operation_ = op; }
void Message::set_src_id(int id) { src_id_ = id; }
void Message::set_dest_id(int id) { dest_id_ = id; }
void Message::set_address(uint64_t addr) { address_ = addr; }
void Message::set_qos(uint8_t q) { qos_ = q; }
void Message::set_size(uint32_t s) { size_ = s; }
void Message::set_num_lines(uint32_t n) { num_lines_ = n; }
void Message::set_start_line(uint32_t sl) { start_line_ = sl; }
void Message::set_cache_line(uint32_t cl) { cache_line_ = cl; }
void Message::set_status(uint32_t st) { status_ = st; }
void Message::set_data(const std::vector<uint32_t>& d) { data_ = d; }

/* --------------------------------------------------------------------------------------------- */

std::string Message::to_string() const {
    auto operation_name = [](Operation k) {
        switch (k) {
            case Operation::READ_MEM:               return "READ_MEM";
            case Operation::WRITE_MEM:              return "WRITE_MEM";
            case Operation::BROADCAST_INVALIDATE:   return "BCAST_INV";
            case Operation::INV_LINE:               return "INV_LINE";
            case Operation::INV_ACK:                return "INV_ACK";
            case Operation::INV_COMPLETE:           return "INV_COMPLETE";
            case Operation::READ_RESP:              return "READ_RESP";
            case Operation::WRITE_RESP:             return "WRITE_RESP";
            case Operation::END:                    return "END";
            case Operation::UNDEFINED:              return "UNDEFINED";
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
