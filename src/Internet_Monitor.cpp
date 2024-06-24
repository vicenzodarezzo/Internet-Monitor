//
// Created by Vicenzo D’Arezzo on 22/06/24.
//

#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <array>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <numeric>
#include <cmath>

#include "../include/Internet_Monitor.h"

// ################################################
// ################################################
// CONSTRUCTOR AND DESTRUCTOR ;
// ################################################
// ################################################

Internet_Monitor::Internet_Monitor(const std::string& filename) {

    try{ loadServerInfo(filename); }
    catch(std::exception  & e){
        DEBUG_PRINT("ERROR DURING SERVERS INPUT FILE READ");
        throw e;
    }

    for (const auto& server : servers) {

        std::string log_file_name = "../logs/" + server.name + "_stats.log";
        auto* logfile = new std::ofstream(log_file_name);

        if (!logfile->is_open()) {
            throw std::runtime_error("Failed to open log file\n");
        } else {
            *logfile << "timestamp,op,pckg_s,pckg_r,pckg_l,conn,min_lat,avg_lat,max_lat,stddev_lat,DNS_flag,TCP_flag\n";
            log_files.push_back(logfile);
            log_files_names.push_back(log_file_name);
            servers_fails.push_back(0);
        }
    }

    signal_log_file = new std::ofstream("../logs/signal_stats.log");
    report_log_file = new std::ofstream("../logs/report.log");

    if(!signal_log_file->is_open() || !report_log_file->is_open()){
        throw std::runtime_error("Failed to open log file\n");
    }else{
        *signal_log_file << "timestamp, op, link_current_quality, link_static_quality, signal_strength\n";
    }
}

Internet_Monitor::~Internet_Monitor() {
    for (auto log_file : log_files) {
        log_file->close();
        delete log_file;
    }
    signal_log_file->close();
    delete signal_log_file;

    report_log_file->close();
    delete report_log_file;
}

// ################################################
// ################################################
// AUX METHODS;
// ################################################
// ################################################

void Internet_Monitor::loadServerInfo(const std::string& file_name) {

    // Opening the servers file and loading the Object's Server list;
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << file_name << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name, ip, url;
        int port;
        if (std::getline(iss, name, ',') && std::getline(iss, ip, ',') &&
            std::getline(iss, url, ',') && iss >> port) {
            DEBUG_PRINT(name << ip << url << port);
            servers.emplace_back(name, ip, url, port);
        }
    }
    file.close();

    DEBUG_PRINT("SERVERS LOADED");
}

void Internet_Monitor::log_write(int operation_id, size_t server_pos,
                     const std::string& msg, bool signal_flag, bool report_flag){

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_time = *std::localtime(&current_time);

    std::stringstream timestamp;
    timestamp << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S");
    timestamp << '.' << std::setfill('0') << std::setw(3) << ms.count();  // Append milliseconds

    std::cout << "Timestamp with milliseconds: " << timestamp.str() << std::endl;



    int id  = static_cast<int>(server_pos);

    if(signal_flag){
        if(signal_log_file->is_open()){
            *(signal_log_file) << timestamp.str() << ","  << msg << std::endl;
            signal_log_file->flush();
        }else{
            throw std::runtime_error("Error on signal log writing");
        }
    }else if(report_flag){
        if(report_log_file->is_open()){
            *(report_log_file) << timestamp.str() << ","  << msg << std::endl;
            report_log_file->flush();
        }else{
            throw std::runtime_error("Error on report log writing");
        }
    }else{
        if(log_files[server_pos]->is_open()){
            *(log_files[server_pos]) << timestamp.str() << "," << operation_id << "," << msg << std::endl;
            log_files[server_pos]->flush();
        }else{
            throw std::runtime_error("Error on log writing - " + std::to_string(id));
        }
    }

}


void Internet_Monitor::log_clear() {

    int log_id = 0;
    std::vector<std::ofstream*> temp_file_vec;

    for (auto& file_ptr : log_files) {

        if (file_ptr && file_ptr->is_open()) {

            file_ptr->close();  // Clean by closing e re-opening it ;
            file_ptr->open(log_files_names[log_id], std::ofstream::out | std::ofstream::trunc);

            if (!file_ptr->is_open()) {
                std::cerr << "Error in cleaning " << log_files_names[log_id] << std::endl;
                throw std::runtime_error("Error re-opening files\n");
            }else{
                *file_ptr << "timestamp,op,pckg_s,pckg_r,pckg_l,conn,min_lat,avg_lat,max_lat,stddev_lat,DNS_flag,TCP_flag\n";
            }

        } else {
            std::cerr << "File in not opened :" << log_files_names[log_id] << std::endl;
            throw std::runtime_error("Error re-opening files\n");
        }

        log_id++;
    }

    if(signal_log_file && signal_log_file->is_open()){
        signal_log_file->close();
        signal_log_file->open("../logs/signal_stats.log", std::ofstream::out | std::ofstream::trunc);

        if(!signal_log_file->is_open()){
            std::cerr << "Error in cleaning " << "../logs/signal_stats.log" << std::endl;
            throw std::runtime_error("Error re-opening files\n");
        }else{
            *signal_log_file << "timestamp, op, link_current_quality, link_static_quality, signal_strength\n";
        }
    }else{
        std::cerr << "File in not opened :" << "../logs/signal_stats.log" << std::endl;
        throw std::runtime_error("Error re-opening files\n");
    }

    if(report_log_file && report_log_file->is_open()){
        report_log_file->close();
        report_log_file->open("../logs/report.log", std::ofstream::out | std::ofstream::trunc);

        if(!report_log_file->is_open()){
            std::cerr << "Error in cleaning " << "../logs/report.log" << std::endl;
            throw std::runtime_error("Error re-opening files\n");
        }

    }else{
        std::cerr << "File in not opened :" << "../logs/report.log" << std::endl;
        throw std::runtime_error("Error re-opening files\n");
    }
}

void Internet_Monitor::exceptions_counter_clear(){
    signal_fails = 0;
    servers_fails.clear();
    for(auto sv : servers) servers_fails.push_back(0);
}

void Internet_Monitor::server_delete(size_t id) {

    if (id >= log_files.size() || id >= log_files_names.size() || id >= servers.size() || id >= servers_fails.size()) {
        DEBUG_PRINT("ERROR: Invalid ID");
        throw std::runtime_error("Invalid ID\n");
    }

    auto& file = log_files[id];

    if (file && file->is_open()) {
        file->close();
        delete file;
    } else {
        DEBUG_PRINT("ERROR: Trying to delete a server file that is already closed");
        throw std::runtime_error("File to be deleted is already closed\n");
    }

    log_files.erase(log_files.begin() + static_cast<long>(id));
    log_files_names.erase(log_files_names.begin() + static_cast<long>(id));
    servers.erase(servers.begin() + static_cast<long>(id));
    servers_fails.erase(servers_fails.begin() + static_cast<long>(id));
}


bool Internet_Monitor::con_exception_treat(Connection_Exception &e) {

    if(e.signal_cause){
        DEBUG_PRINT("SIGNAL FAIL EXCEPTION");
        log_write(3, -1, "[CONNECTION EXCEPTION] Signal Fail", false, true);
        signal_fails++;
    }else{
        std::stringstream msg;
        msg <<  "[CONNECTION EXCEPTION] Server Fail: " << servers[e.server].name ;
        log_write(3, -1, msg.str(), false, true);
        DEBUG_PRINT("SERVER FAIL EXCEPTION");
        servers_fails[e.server]++;
    }

    if(signal_fails > SIGNAL_FAIL_THRESHOLD){
        DEBUG_PRINT("FATAL SIGNAL FAIL");
        log_write(3, -1, "[FATAL] Signal lost in wlan0", false, true);
        throw Connection_Exception("Wi-fi Signal lost in wlan0\n", true, -1);
    }

    // Calculating the mean of the server fails and searching for an
    // outlier by looking if the value differ more than 2 standard dev.
    // from the mean number of fails.

    double sum = std::accumulate(servers_fails.begin(), servers_fails.end(), 0.0);

    if(sum >= SERVER_FAIL_THRESHOLD){

        DEBUG_PRINT("INITIATING THE SERVERS ANALYSIS");

        // Calculating the mean of the server fails and searching for an
        // outlier by looking if the value differ more than 2 standard dev.
        // from the mean number of fails.

        size_t server_id_rem = -1;
        double mean = sum / static_cast<double>(servers_fails.size());
        double sq_sum = std::inner_product(servers_fails.begin(), servers_fails.end(), servers_fails.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / static_cast<double>(servers_fails.size()) - mean * mean);

        for (size_t i = 0; i < servers_fails.size(); ++i) {
            if (std::abs(servers_fails[i] - mean) > 2 * stdev) {
                server_id_rem = i;
                break;
            }
        }

        // If there are an outlier, the monitor just drops the server until the new
        // log refreshing. If there are not, the monitor shut down the system
        // interpretation this as a too low wi-fi signal.

        if(server_id_rem != -1){
            std::stringstream msg;
            msg << "[OUTLIER CASE] " << servers[server_id_rem].name << "- deleting the server";
            DEBUG_PRINT(msg.str());
            log_write(4, -1, msg.str(), false, true);

            server_delete(server_id_rem);
            return true;
        }
        else{
            DEBUG_PRINT("FATAL LOW CONNECTIVITY");
            log_write(4, -1,
                      "[FATAL] Too much connection exceptions without losing signal - low connectivity",
                      false, true);
            throw Connection_Exception("Wi-fi Signal too low for conectivity\n", false, -1);
        }
    }
    return false;
}


// ################################################
// ################################################
// FUNCTIONALITIES METHODS;
// ################################################
// ################################################

void Internet_Monitor::pingServer(size_t server_id) {

    std::string ip = servers[server_id].ip;

    std::string command = "ping -c 2 " + ip + " 2>&1";
    std::array<char, 128> buffer{};
    std::string result;

    // LOG PING VAR
    bool conn_flag = true;
    int packages_trans = -1, packages_rec = -1, packages_loss = -1;
    float connectivity = -1;
    float min_latency = -1, avg_latency = -1, max_latency = -1, mdev_latency = -1;

    // FORKING THE PROCESS IN ORDER TO PING TARGET SERVER
    DEBUG_PRINT("FORKING TO PING");

    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        DEBUG_PRINT("Failed to run ping command." << std::endl);
        throw std::runtime_error("the pipe and forking failed!\n");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();

    DEBUG_PRINT("PING RESULT: " << result << std::endl);

    // EXTRACTING PING INFORMATION:

    // -> [PACK_COUNT]
    // -> --- 8.8.8.8 ping statistics ---
    // -> 4 packets transmitted, 4 packets received, 0.0% packet loss
    size_t pos = result.find("ping statistics ---\n");


    if (pos != std::string::npos) {
        pos += 20;
        std::string pingStats = result.substr(pos);

        DEBUG_PRINT("Informacoes de pacotes: " << pingStats << std::endl);

        std::istringstream iss(pingStats);
        std::string token;
        iss >> packages_trans >> token >> token >> packages_rec >> token >> token >> packages_loss;

        connectivity = ( static_cast<float>(packages_rec) / static_cast<float>(packages_trans) ) * 100;
        if(connectivity == 0) conn_flag = false;

        DEBUG_PRINT("S: " <<  packages_trans << "/ R: " << packages_rec << "/ L: " << packages_loss << "/ C: " << connectivity);

    }else{
        DEBUG_PRINT("FAILING IN MATCHING THE PING RETURN");
        conn_flag = false;
    }

    // -> [LAT_STATS]
    // -> round-trip min/avg/max/stddev = 12.049/17.427/20.588/3.218 ms (mac)
    // -> rtt min/avg/max/mdev = 30.173/30.742/31.052/0.346 ms (linux - ubuntu 20.04)

    pos = result.find("min/avg/max/mdev");

    if (pos != std::string::npos) {
        std::string latencyStats = result.substr(pos);

        DEBUG_PRINT("LAT STATS: " << latencyStats << std::endl);

        size_t start = latencyStats.find_first_of("0123456789");

        if (start != std::string::npos) {

            DEBUG_PRINT("PARSING LAT. STATS");

            latencyStats = latencyStats.substr(start);

            std::stringstream ss(latencyStats);
            char delimiter;
            ss >> min_latency >> delimiter >> avg_latency >> delimiter >> max_latency >> delimiter >> mdev_latency;
        }else{
            throw std::runtime_error("Error parsing the latency stats returned from forked process pipe\n");
        }

        DEBUG_PRINT("L_MIN: " << min_latency << " ms");
        DEBUG_PRINT("L_MEAN: " << avg_latency << " ms");
        DEBUG_PRINT("L_MAX " << max_latency << " ms");
        DEBUG_PRINT("L_STD " << mdev_latency << " ms");

    } else {

        DEBUG_PRINT("Failed to get the ping return from the forked process pipe [LAT_STATS]");
        std::stringstream msg;
        msg << "\t\tpos after result.find: " << pos;
        DEBUG_PRINT(msg.str());
        conn_flag = false;
    }

    std::stringstream msg;

    msg << packages_trans << "," << packages_rec << "," << packages_loss << "," << connectivity << ","
        << min_latency << "," << avg_latency << "," << max_latency << "," << mdev_latency
        << "," << -2 << "," << -2;

    log_write(0, server_id, msg.str(), false, false);

    if(!conn_flag){
        std::stringstream msg_1;
        msg_1 << "[PING FAIL] Server " << servers[server_id].name << "\n\t\t[PING OUTPUT] " << result;
        log_write(0, -1, msg_1.str(), false, true);
        throw Connection_Exception("Fail in receiving the Ping return\n", false, server_id);
    }
}


// ################################################
// ################################################


void Internet_Monitor::checkSignalStrength() {

    bool conn_flag = true;
    double link_static_quality = -1, link_current_quality = -1, signal_level = -1.0;

    std::stringstream command;
    
    command << "iwconfig " << wifi_dev << " | grep 'Link Quality'";

    DEBUG_PRINT("FORKING TO EXECUTE iwconfig");

    FILE* pipe = popen(command.str().c_str() , "r");
    if (!pipe) {
        throw std::runtime_error("Pipe from forked process was fail to open\n");
    }

    char buffer[128];
    std::string result;
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) result += buffer;
    }
    pclose(pipe);

    DEBUG_PRINT("iwconfig return: " << result);

    // EXTRACTING PING INFORMATION:

    // -> [LINK_Q]
    // -> "Link Quality=xx/xx"
    size_t pos = result.find("Link Quality=");

    if (pos != std::string::npos) {

        std::string qualityStr = result.substr(pos + 13, 4);
        std::istringstream iss(qualityStr);

        char tmp_char; // for extracting the delimiter in the returned msg;

        if (!(iss >> link_current_quality >> tmp_char  >> link_static_quality)) {
            throw std::runtime_error("Failed to parse Link Quality after finding the ""Link Quality=""\n");
        }

        link_current_quality = link_static_quality / link_static_quality * 100.0; // relative link quality;
        link_static_quality = link_static_quality / 70 * 100.0; // absolute link quality - 70 is the max value in iwconfig;

        DEBUG_PRINT("REL_LINK_Q: " << link_current_quality);
        DEBUG_PRINT("ABS_LINK_Q: " << link_static_quality);

    }else{
        DEBUG_PRINT("Failed to parse the returned msg from forked process pipe [LINK_Q]\n");
        conn_flag = false;
    }

    // -> [SIGNAL LV] ;
    // -> "Signal level=xxx/xxx" ;
    pos = result.find("Signal level=");

    if (pos != std::string::npos) {

        std::string signalStr = result.substr(pos + 13, 3);
        std::istringstream iss(signalStr);

        if (!(iss >> signal_level)) {
            throw std::runtime_error("Failed to parse Signal Level after finding the ""Signal level=""\n");
        }
    }else{
        std::stringstream msg;
        msg << "[IWCONFIG FAIL]\n\t\t[IWCONFIG OUTPUT] " << result;
        DEBUG_PRINT("Failed to parse the returned msg from forked process pipe [SIGNAL LV]\n");
        log_write(1, -1, msg.str(), false, true);
        conn_flag = false;
    }

    DEBUG_PRINT("SENDING MSG TO LOG WRITE");

    std::stringstream msg;
    msg << link_current_quality << "," << link_static_quality << "," << signal_level;

    for(int i = 0;  i < log_files.size(); i++) log_write(1, -1, msg.str(), true, false);

    if(!conn_flag){
        throw Connection_Exception("Fail in receisving the iwconfig return\n", true, -1);
    }
}


// ################################################
// ################################################


void Internet_Monitor::check_DNS_and_PORT(size_t server_id) {

    int x; // DNS success value ;
    int y = 0; // Open Port success value ;
    bool conn_flag = true;
    std::string serverName = servers[server_id].name;
    int port = servers[server_id].port;

    // DNS RESOLUTION USING THE "socket.h" AND "netinet/in.h" STRUCT ;

    struct addrinfo hints{}, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    //  IPv4 || IPv6 ;
    hints.ai_socktype = SOCK_STREAM; // TCP only ;

    // Tries to resolve DNS ;
    x = (int) getaddrinfo(serverName.c_str(), nullptr, &hints, &res) == 0;

    if(x){ // DNS was successful ;

        // TCP CONNECTION ESTABLISHMENT ;
        // tries to establish a connection through each of the possible addresses ;
        struct addrinfo *p;

        for (p = res; p != nullptr && (y == 0); p = p->ai_next) {

            int sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sock < 0) continue;

            if (p->ai_family == AF_INET) {
                auto ipv4 = (struct sockaddr_in *) p->ai_addr;
                ipv4->sin_port = htons(port);
            } else {
                auto ipv6 = (struct sockaddr_in6 *) p->ai_addr;
                ipv6->sin6_port = htons(port);
            }

            y = (int) connect(sock, p->ai_addr, p->ai_addrlen) == 0;
            close(sock);
        }
    }

    freeaddrinfo(res);

    std::stringstream report_msg;

    if(x == 0){
        x = -1;
        y = x;
        conn_flag = false;
        report_msg << "[DNS ERROR] DNS resolution failed for " << serverName;
        log_write(2, -1, report_msg.str(), false, true);

    }else if(y == 0){
        y = -1;
        report_msg << "[TCP ERROR] Unable to establish connection to " << serverName << " in port" << port;
        log_write(2, -1, report_msg.str(), false, true);

    }

    std::stringstream msg;

    msg  << -2 << "," << -2 << "," << -2 << "," << -2 << "," << -2 << "," << -2 << ","
        << -2 << "," << -2 << "," << x << "," <<  y;
    log_write(2, server_id, msg.str(), false, false);

    if(!conn_flag){
        DEBUG_PRINT("Failed to parse the returned msg from forked process pipe [SIGNAL LV]\n");
        throw Connection_Exception("Error in DNS solving\n", false, server_id);
    }
}


// ################################################
// ################################################


void Internet_Monitor::monitor() {

    auto log_periodic_time = std::chrono::steady_clock::now();
    auto con_ex_periodic_time = std::chrono::steady_clock::now();

    while(true){

        auto current_time = std::chrono::steady_clock::now();

        std::chrono::duration<double> log_interval = current_time - log_periodic_time;
        std::chrono::duration<double> con_ex_interval = current_time - con_ex_periodic_time;


        if(log_interval.count() >= CLEAN_PERIOD_IN_SECS){
            DEBUG_PRINT("\n\n######### LOGS CLEANED #########\n\n");
            log_clear();
            log_periodic_time = current_time;
            log_write(4, -1, "[CLEAN] Log files cleaned", false, true);
        }

        if(con_ex_interval.count() >= CLEAN_EX_CON_COUNTER_IN_SECS){
            DEBUG_PRINT("\n\n######### CONNECTIONS EXCEPTIONS CLEANED #########\n\n");
            exceptions_counter_clear();
            con_ex_periodic_time = current_time;
            log_write(4, -1, "[CLEAN] exceptions counters cleaned", false, true);
        }

        try{ checkSignalStrength(); }
        catch(Connection_Exception & e){
            DEBUG_PRINT("CONNECTION EXCEPT CAPTURED IN SIGNAL TESTING" << e.what());
            con_exception_treat(e);
        }catch(const std::exception & e){

            DEBUG_PRINT("EXCEPT CAPTURED IN SIGNAL TESTING" << e.what());

        }

        size_t server_id = 0;

        while(server_id < servers.size()){

            bool rem_server_flag = false;

            // if the current server in analysis was removed in the
            // treating of connection exception, the iteration needs to
            // jump to the next one. In case of the Ping Analysis,
            // we just interrupt this iteration and go the next one. In the
            // case of the second one, we maintain the id in due to the 1
            // server smaller list.

            auto server = servers[server_id];

            try{
                pingServer(server_id);
            }
            catch(Connection_Exception & e){
                DEBUG_PRINT("CONNECTION EXCEPT CAPTURED IN SIGNAL TESTING" << e.what());
                rem_server_flag = con_exception_treat(e);
            }catch( const std::exception & e){
                DEBUG_PRINT("EXCEPT CAPTURED IN SERVER LOOP" << e.what());

            }

            if(rem_server_flag) continue;

            if(server.port != -1){
                try{
                    check_DNS_and_PORT(server_id);
                }catch(Connection_Exception & e){
                    DEBUG_PRINT("CONNECTION EXCEPT CAPTURED IN DNS SOLVING" << e.what());
                    rem_server_flag = con_exception_treat(e);

                }catch( const std::runtime_error & e){
                    DEBUG_PRINT("EXCEPT CAPTURED IN DNS & TCP CHECKING" << e.what());

                }
            }

            if(!rem_server_flag) server_id++;
        }
    }
}
