#include "../include/components/PE.h"

PE::PE(int id, uint8_t qos)
    : id_(id), qos_(qos) {}

int PE::get_id() const {
    return id_;
}

uint8_t PE::get_qos() const {
    return qos_;
}
