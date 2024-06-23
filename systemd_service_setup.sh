#!/bin/bash

CURRENT_DIR=$(pwd)
CURRENT_USER=$(whoami)

echo "DIR: ${CURRENT_DIR}\n"
echo "USR: ${CURRENT_DIR}\n"

# Catches the service directory and creates the new service file ;

SERVICE_FILE="/etc/systemd/system/internet_monitor.service"

# Defines the service file ;
cat <<EOF | sudo tee $SERVICE_FILE > /dev/null
[Unit]
Description=Internet Monitor using wifi NIC
After=network.target

[Service]
Type=simple


ExecStart=${CURRENT_DIR}/execute.sh
WorkingDirectory=${CURRENT_DIR}
Restart=on-success
User=${CURRENT_USER}
Group=${CURRENT_USER}
StandardOutput=journal
StandardError=journal
TimeoutStopSec=30

[Install]
WantedBy=multi-user.target
EOF

# Allows the service file to execute and be read by the system ;
sudo chmod 644 $SERVICE_FILE

sudo systemctl daemon-reload
sudo systemctl enable internet_monitor.service
sudo systemctl start internet_monitor.service

# Verifica o status do serviço ;
sudo systemctl status internet_monitor.service

# Exibe os logs do serviço ;
echo "SERVICE OUTPUT:"
sudo journalctl -u internet_monitor.service -n 50

echo "WIFI_MONITOR SERVICE STATUS: INITIATED"