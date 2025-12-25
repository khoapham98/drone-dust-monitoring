#!/bin/bash

# Configuration
SERVICE_NAME="drone.service"
SOURCE_PATH="$(pwd)/scripts/$SERVICE_NAME"
DEST_PATH="/etc/systemd/system/$SERVICE_NAME"
CURRENT_DIR="$(pwd)"
CURRENT_USER=$(whoami)

echo "Installing $SERVICE_NAME..."

if [ ! -f "$CURRENT_DIR/build/bin/app" ]; then
    echo "[ERROR] Executable 'app' not found!"
    echo "Please build your project first by running: make"
    exit 1
fi
echo "[1/5] Executable found. Proceeding with installation..."

sed -i "s|WorkingDirectory=.*|WorkingDirectory=$CURRENT_DIR|" "$SOURCE_PATH"
sed -i "s|ExecStart=.*|ExecStart=$CURRENT_DIR/build/bin/app|" "$SOURCE_PATH"
sed -i "s|User=.*|User=$CURRENT_USER|" "$SOURCE_PATH"
echo "[2/5] Updating paths and User ($CURRENT_USER) in service file: DONE"

sudo cp "$SOURCE_PATH" "$DEST_PATH"
echo "[3/5] Copying service file to system: DONE"

sudo systemctl daemon-reload
# enable auto-boot
# sudo systemctl enable "$SERVICE_NAME" > /dev/null 2>&1
echo "[4/5] Systemd reload: DONE"

sudo systemctl reset-failed "$SERVICE_NAME"
sudo systemctl start "$SERVICE_NAME"
if [ $? -eq 0 ]; then
    echo "[5/5] Starting service: DONE"
    echo "------------------------------------------"
    echo "[SUCCESS] $SERVICE_NAME is now running."
    echo "Commands to manage your service:"
    echo "  - Check status: systemctl status $SERVICE_NAME"
    echo "  - Follow logs:  journalctl -u $SERVICE_NAME -f"
    echo "  - Stop service: sudo systemctl stop $SERVICE_NAME"
    echo "------------------------------------------"
else
    echo "[ERROR] Failed to start service."
    exit 1
fi