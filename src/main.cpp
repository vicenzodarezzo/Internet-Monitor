#include "../include/Internet_Monitor.h"

int main() {

    auto * monitor = new Internet_Monitor("../servers.txt");
    try{
        monitor->monitor();
    }catch(Connection_Exception & e){
        delete monitor;
        std::cerr << e.what();
        return 0;
    }catch(std::exception & e){
        delete monitor;
        std::cerr << e.what();
        return 1;
    }

    return 0;
}
