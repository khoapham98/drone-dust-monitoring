# Drone Application to Measure and Map Fine Dust in Ho Chi Minh City

## Features
* **AQI Monitoring:** Captures PM2.5 data from the PMS7003 sensor and converts raw concentrations into the Air Quality Index (AQI).
* **Telemetry Data:** Extracts precise GPS coordinates from Pixhawk flight controller via MAVLink protocol.
* **LTE Communication:** Utilizes the A7680C SIM module to transmit data to the server via HTTP requests.
* **Visualization:** Streams spatial data to a web dashboard for live pollution mapping in Ho Chi Minh City.
* **Hardware Support:** Compatible with Raspberry Pi and BeagleBone Black.

(_Web dashboard link will be updated soon_)

## Getting Started
### 1. Hardware Configuration
To select your platform, set the target board by adjusting the macro values in `src/device_setup.h`:
```c
/* select board */
#define BBB    0
#define RPI    1
```

### 2. Build and Run
Compile the source code:
``` Bash
make
```
Execute the application:
``` Bash
make run
```

### 3. Service Installation
Grant execution rights to the deployment script and install the application as a background system service:
``` Bash
sudo chmod +x scripts/setup_service.sh
make install-service
```

### 4. Clean Build
Clean the build directory:
``` Bash
make clean
```
