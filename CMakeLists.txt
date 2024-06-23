cmake_minimum_required(VERSION 3.28)
project(wifi_monitor)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

add_executable(wifi_monitor
        src/main.cpp
        src/Internet_Monitor.cpp
        src/Server_Info.cpp
        src/Connection_Exception.cpp
        include/Connection_Exception.h
)

target_include_directories(wifi_monitor PRIVATE include)

