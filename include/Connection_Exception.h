//
// Created by Vicenzo D’Arezzo on 22/06/24.
//

#ifndef WIFI_MONITOR_CONNECTION_EXCEPTION_H
#define WIFI_MONITOR_CONNECTION_EXCEPTION_H

#include <stdexcept>
#include <string>

class Connection_Exception : public std::exception {

    private:
    std::string message;


    public:
    bool signal_cause;
    size_t server;

    explicit Connection_Exception(std::string  msg, bool signal_flag, size_t server_id);
    virtual const char * what() const noexcept override;
};


#endif //WIFI_MONITOR_CONNECTION_EXCEPTION_H
