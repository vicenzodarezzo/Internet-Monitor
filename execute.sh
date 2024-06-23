#!/bin/bash

# Creat log directory
if [ ! -d "logs" ]; then
    mkdir logs
    echo "Diretório 'logs' criado."
fi

# install iwconfig
if ! command -v iwconfig &> /dev/null; then
    echo "Comando iwconfig não encontrado. Instalando..."
    sudo apt-get update
    sudo apt-get install wireless-tools
fi

cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./wifi_monitor
