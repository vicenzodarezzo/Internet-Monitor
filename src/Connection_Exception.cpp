//
// Created by Vicenzo D’Arezzo on 22/06/24.
//

#include <utility>

#include "../include/Connection_Exception.h"

Connection_Exception::Connection_Exception(std::string msg, bool signal_flag, size_t server_id)
    : message(std::move(msg)) {
    signal_cause = signal_flag;
    server = server_id;
}

const char * Connection_Exception::what() const noexcept {
    return message.c_str();
}