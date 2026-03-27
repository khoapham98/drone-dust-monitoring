# Drone Dust Monitoring

## Overview
This project implements an embedded system integrated on a drone for PM2.5 monitoring and mapping.

The system reads data from a dust sensor, converts it to AQI, combines it with GPS data from Pixhawk (MAVLink), and transmits it to a server via cellular (HTTP/MQTT).

## Demo Video
[Watch demo video on YouTube](https://youtu.be/ysVYPlQU1Qo)

## Hardware
- ESP32  
- Dust sensor (PMS7003)
- GPS module (Holybro M10)  
- Flight controller (Pixhawk 6C Mini)  
- SIM module (A7680C)  

## MAVLink Configuration
1. Install and open **Mission Planner**
2. Connect Pixhawk to your computer via USB  
3. Select the COM port and click **Connect**  
4. Go to: **CONFIG → Full Parameter List**
5. Set the following parameters:
    ```
    SR1_POSITION = 1
    SR1_EXT_STAT = 1
    ```
6. Click **Write Params**

## Getting Started

### Requirements
- ESP-IDF v5.3+

### Build & Flash
```bash
idf.py build
idf.py flash
idf.py monitor
```

Flash and monitor in one step:
```bash
idf.py -p PORT flash monitor
```