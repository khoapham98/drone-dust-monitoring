# UART Pin Configuration on BeagleBone Black

This document summarizes the commands required to configure UART pins on the BeagleBone Black (BBB), along with a complete list of UART modules and their associated pins.

---

## 1. Check Current Pin Configuration
```
config-pin -q P9_XX
```
This command displays the current mode of the selected pin (gpio, uart, spi, pwm, etc.)

## 2. Set Pin to UART Mode
```
sudo config-pin P9_XX uart
```
Use this command to switch a pin into UART mode.

## 3. UART Pin Mapping on BBB
| Function      | Pin      |
|---------------|----------|
| UART1_TXD     | P9_24    |
| UART1_RXD     | P9_26    |
| UART2_TXD     | P9_21    |
| UART2_RXD     | P9_22    |
| UART4_RXD     | P9_11    |
| UART4_TXD     | P9_13    |
| UART5_TXD     | P8_37    |
| UART5_RXD     | P8_38    |
