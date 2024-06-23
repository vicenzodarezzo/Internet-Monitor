//
// Created by Vicenzo D’Arezzo on 22/06/24.
//

#ifndef WIFI_MONITOR_SERVER_H
#define WIFI_MONITOR_SERVER_H

#include <iostream>
#include <utility>

#endif //WIFI_MONITOR_SERVER_H

// Classe Server_Info
class Server_Info {
public:
    std::string name;
    std::string ip;
    std::string url;
    int port;

    Server_Info(std::string  n, std::string  i, std::string  u, int p);
};

