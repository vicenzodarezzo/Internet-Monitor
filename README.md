# Basic Internet Monitoring Application for Wi-FI based Linux Systems

 A project for an Internet connectivity monitoring system, based on the Wi-Fi network interface.

For monitoring, it uses the Ping command to check reachability to servers via ICMP packets; the iwconfig command to verify signal and link quality; and DNS resolution attempts and TCP connections to verify server availability.
Collected Data

The collected data can be found in the 'logs' folder created by the process, where files about each of the monitored servers are located, a file describing signal quality, and a generated report file where problems and their handling flow are described in a textual manner.
Running the system:

To run the system, simply give permission to the execution script 'execute.sh' and run it as an administrator.

bash

chmod +x execute.sh
sudo ./execute.sh

Adapting the project

Important definitions for the system's operation are found in the header file 'include/Internet_Monitor.h'. If necessary, adapt them to your project.

    CLEAN_PERIOD_IN_SECS: refresh period for log files;
    CLEAN_EX_CON_COUNTER_IN_SECS: refresh period for connection exceptions;
    SIGNAL_FAIL_THRESHOLD: number of exceptions required to diagnose connection drop;
    SERVER_FAIL_THRESHOLD: number of exceptions required to temporarily disconnect a server;
    std::string wifi_dev: network interface used.

Adding servers

To add coverage for more servers, just include them in 'servers.txt', following the pattern:

    name, IP, url, port;
        if you don't want to add the port, set the value to -1.

Dependencies

    C++11
    CMake 3.28
    Linux operating system
