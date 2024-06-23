#include <utility>

#include "../include/Server_Info.h"

Server_Info::Server_Info(std::string  n, std::string  i, std::string  u, int p)
        : name(std::move(n)), ip(std::move(i)), url(std::move(u)), port(p) {}
