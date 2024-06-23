#include "../include/Internet_Monitor.h"

int main() {

    auto * monitor = new Internet_Monitor("../servers.txt");
    try{
        monitor->monitor();
    }catch(std::exception & e){
        delete monitor;
        std::cerr << e.what();
    }

    return 0;
}
