//
// Created by Vicenzo D’Arezzo on 21/06/24.
//
#ifndef INTERNET_MONITOR_H
#define INTERNET_MONITOR_H

#include <vector>
#include <string>
#include "Server_Info.h"
#include "../include/Connection_Exception.h"

#define CLEAN_PERIOD_IN_SECS 240
#define CLEAN_EX_CON_COUNTER_IN_SECS 15
#define SIGNAL_FAIL_THRESHOLD 5
#define SERVER_FAIL_THRESHOLD 10


//#ifdef DEBUG
    #define DEBUG_PRINT(x) std::cout << "DEBUG: " << x << std::endl
//#else
//    #define DEBUG_PRINT(x)
//#endif




class Internet_Monitor {

private:
    std::vector<Server_Info> servers;
    std::vector<std::ofstream*> log_files;
    std::ofstream * signal_log_file;
    std::ofstream * report_log_file;
    std::vector<std::string> log_files_names;
    std::vector<int> servers_fails;
    std::string wifi_dev = "wlan0";
    int signal_fails = 0;

public:

    /**
 * @brief Constructor for the Internet_Monitor class.
 *
 * Initializes an instance of Internet_Monitor using server information loaded from a file.
 *
 * @param filename A constant reference to a string containing the filename from which
 *        server information will be loaded.
 *
 * @throws std::exception If an error occurs during the loading of server information,
 *         the exception is caught, and an error message is printed. The exception is
 *         then rethrown to propagate the error.
 *
 * @throws std::runtime_error If a log file associated with any server cannot be opened,
 *         a runtime_error is thrown indicating the failure.
 */

    explicit Internet_Monitor(const std::string& filename);


/**
 * @brief Destructor for the Internet_Monitor class.
 *
 * Cleans up resources associated with the Internet_Monitor instance, including
 * closing and deleting all opened log files.
 */
    ~Internet_Monitor();


/**
 * @brief Performs a ping operation to measure connectivity and latency to a specified IP address.
 *
 * Executes a ping command to the given IP address, captures the output, and extracts statistics
 * regarding packet transmission and latency. The obtained statistics are then logged using the
 * `log_write` function. If the Ping command was not successfully executed, it writes -1 value in
 * the log file;
 *
 * Assumes the Ping Response as:
 *  PING 8.8.8.8 (8.8.8.8): 56 data bytes
 *  64 bytes from 8.8.8.8: icmp_seq=0 ttl=117 time=12.465 ms
 *  64 bytes from 8.8.8.8: icmp_seq=1 ttl=117 time=17.903 ms
 *  64 bytes from 8.8.8.8: icmp_seq=2 ttl=117 time=20.691 ms
 *  64 bytes from 8.8.8.8: icmp_seq=3 ttl=117 time=20.344 ms
 *
 *  --- 8.8.8.8 ping statistics ---
 *  4 packets transmitted, 4 packets received, 0.0% packet loss
 *  round-trip min/avg/max/stddev = 12.465/17.851/20.691/3.290 ms
 *
 *  for adapting this interpretation, change the command string definition in the method def.
 *
 * @param server_id The ID used to identify the log entry where ping statistics will be logged.
 *
 * @throws std::runtime_error If there is an error during the execution of the ping command,
 *         or if parsing the output fails, a runtime_error is thrown with an appropriate error message.
 * @throws Connection_exception : If there are any indicative of lost connection.
 */
    //OPERATION CODE: 0
    void pingServer(size_t server_id);


/**
 * @brief Checks and logs the signal strength and link quality of the WLAN interface.
 * 
 * - Uses the defined device in the class declaration (wif_dev) as the wi-fi network interface card ;
 *
 * Executes the 'iwconfig wlan0 | grep 'Link Quality'' and 'iwconfig {WIFI_DEV} | grep 'Signal level''
 * commands to retrieve current link quality and signal level information from the WLAN interface.
 * Parses the output to extract relative and absolute link quality values, as well as signal level,
 * and logs these values into the associated log files using the `log_write` function.
 *
 * If the command encounter any difficulties, it writes the -1 value in log files, indicating
 * the connection error.
 *
 * @throws std::runtime_error If there is an error in executing the commands, parsing their output,
 *         or logging the information, a runtime_error is thrown with an appropriate error message.
 * @throws Connection_exception : If there are any indicative of lost connection.
 */
    // OPERATION CODE: 1
    void checkSignalStrength();


    /**
 * @brief Checks DNS resolution and port availability for a specified server.
 *
 * Attempts to resolve the DNS for the given server name using the `getaddrinfo` function.
 * If DNS resolution is successful, attempts to establish a TCP connection to the server
 * on the specified port using `socket`, `connect`, and `close` functions. Logs the results
 * of DNS resolution and port availability (success or failure) using the `log_write` function.
 * If the method encounters difficulties to solve the DNS or accessing the port, writes -1 in
 * the log file, indicating the situation.
 *
 *
 * @param server_id The ID used to identify the log entry where DNS and port check results will be logged.
 * @throws std::runtime_error If there is an error in DNS resolution, TCP connection establishment,
 *         or logging the information, a runtime_error is thrown with an appropriate error message.
 * @throws Connection_exception : If there are any indicative of lost connection.
 */
    // OPERATION CODE: 2
    void check_DNS_and_PORT(size_t server_id);

/**
 * @brief Method to continuously monitor internet servers and handle exceptions.
 *
 * Monitors the status of internet servers, periodically cleaning logs and resetting exception counters.
 * Handles runtime errors and connection exceptions during server checks and responds accordingly.
 *
 * This method performs the following actions in a continuous loop:
 * - Retrieves the current time using std::chrono::steady_clock.
 * - Checks if the log cleaning interval (CLEAN_PERIOD_IN_SECS) has elapsed since the last log cleaning.
 *   - If true, clears log files using log_clear(), updates the last log cleaning time, and logs the action.
 * - Checks if the connection exceptions cleaning interval (CLEAN_EX_CON_COUNTER_IN_SECS) has elapsed.
 *   - If true, clears exception counters using exceptions_counter_clear(), updates the last cleaning time, and logs the action.
 * - Attempts to check Wi-Fi signal strength using checkSignalStrength().
 *   - Catches std::runtime_error and logs the exception.
 *   - Catches Connection_Exception and handles it using con_exception_treat().
 * - Iterates through each server in the servers list:
 *   - Calls pingServer(server_id) to ping the server and handles exceptions.
 *     - Catches std::runtime_error and logs the exception.
 *     - Catches Connection_Exception and handles it using con_exception_treat(), potentially removing the server.
 *   - If the server has a specified port (not -1), calls check_DNS_and_PORT(server_id).
 *     - Catches Connection_Exception and handles it using con_exception_treat(), potentially removing the server.
 *
 * Servers that are removed during exception handling are skipped in subsequent iterations.
 *
 * @throws std::runtime_error If a runtime error occurs during signal strength checking or server pinging, it is logged.
 * @throws Connection_Exception If a fatal Wi-Fi signal loss or low connectivity is detected during monitoring, appropriate actions are taken.
 */
    void monitor();

private:

    /**
 * @brief Loads server information from a specified file into the Internet_Monitor instance.
 *
 * Opens the specified file, reads server details line by line, and populates the
 * Internet_Monitor's server list with the parsed information.
 *
 * @param file_name A constant reference to a string containing the name of the file
 *        from which server information is to be loaded.
 *
 * @details The file should contain lines formatted as 'name,ip,url,port', where each
 *        field represents a server's name, IP address, URL, and port number respectively.
 *        Each line is parsed to create a Server object, which is then added to the
 *        Internet_Monitor's server list.
 *
 * @note If the specified file cannot be opened, an error message is printed to standard
 *       error, and the function returns without modifying the server list.
 */
    void loadServerInfo(const std::string& filename);


    /**
 * @brief Writes a log message to the specified server's log file.
 *
 * Writes a formatted log entry containing an operation ID and a message to the log
 * file associated with the server at the specified position in the `log_files` vector.
 * If the log file is not open or cannot be accessed, a runtime_error is thrown.
 * In case of signal or report logs, it writes in the specific file;
 *
 * @param operation_id An integer representing the ID of the operation being logged.
 * @param server_pos The position of the server in the `log_files` vector where the
 *        log message should be written.
 * @param msg A constant reference to a string containing the message to be logged.
 * @param signal_flag Indicator to the signal case;
 * @param report_flag Indicator to the report case;
 *
 * @throws std::runtime_error If the log file associated with the specified server
 *         position is not open or cannot be accessed, a runtime_error is thrown
 *         with an error message indicating the server position.
 */
    void log_write(int operation_id, size_t server_pos, const std::string& msg,
                   bool signal_flag, bool report_flag);


/**
 * @brief Clears (truncates) all log files associated with the Internet_Monitor instance.
 *
 * Closes each open log file, reopens it in truncate mode (which clears existing content),
 * and verifies successful reopening. If any file cannot be reopened or is not open initially,
 * an error message is printed to standard error, and a runtime_error is thrown.
 *
 * @throws std::runtime_error If any log file cannot be reopened or is not open initially,
 *         a runtime_error is thrown with an error message indicating the specific file.
 */
    void log_clear();


/**
 * @brief Method to clear the exception counters.
 *
 * Resets the signal failure count and clears the server failure counts.
 * Initializes the server failure counts to zero for all servers.
 */
    void exceptions_counter_clear();


    /**
 * @brief Method to delete a server and its associated log file.
 *
 * Removes the server and its associated log file from the monitoring system. Ensures that all related resources are properly closed and deleted.
 *
 * @param id The index of the server to be deleted.
 *
 * @throws std::runtime_error If the provided ID is invalid or if the log file is already closed.
 *
 * This method performs the following steps:
 * - Validates the provided ID against the sizes of the server, log files, log file names, and server failure counts vectors.
 * - If the ID is invalid, logs an error message and throws a runtime_error.
 * - If the log file associated with the server is open, closes and deletes the log file.
 * - If the log file is already closed, logs an error message and throws a runtime_error.
 * - Removes the server, log file, log file name, and failure count from their respective vectors.
 */
    void server_delete(size_t id);


    /**
 * @brief Method to handle connection exceptions.
 *
 * Processes the given Connection_Exception and determines the appropriate action
 * based on the type of exception and the current failure counts.
 *
 * @param e A reference to the Connection_Exception object to be processed.
 *
 * @return True if a server was identified as an outlier and deleted; false otherwise.
 *
 * @throws Connection_Exception If the signal failure threshold is exceeded
 *      or if the overall server failure indicates a fatal low connectivity.
 *
 * This method performs the following steps:
 * - Checks if the exception is caused by a signal failure or a server failure.
 * - Increments the appropriate failure counter (signal_fails or servers_fails).
 * - If the signal failure count exceeds SIGNAL_FAIL_THRESHOLD, logs a fatal signal failure and throws a Connection_Exception indicating Wi-Fi signal loss.
 * - Calculates the total number of server failures and checks if it exceeds SERVER_FAIL_THRESHOLD.
 * - If the server failure threshold is exceeded, calculates the mean and standard deviation of server failures to identify any outliers.
 * - If an outlier server is found (failure count differing more than 2 standard deviations from the mean), deletes the server and returns true.
 * - If no outlier server is found and overall connectivity is too low, logs a fatal low connectivity error and throws a Connection_Exception indicating low Wi-Fi signal.
 * - Returns false if no outlier server was found and no fatal condition was met.
 */
    bool con_exception_treat( Connection_Exception & e);

};



#endif // INTERNET_MONITOR_H
