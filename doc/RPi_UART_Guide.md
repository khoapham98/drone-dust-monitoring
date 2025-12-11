# Enable UART on Raspberry Pi (Ubuntu & Raspberry Pi OS)

# 1. Enable UART

## Ubuntu
```
sudo vi /boot/firmware/usercfg.txt
```
### Add the following lines:
```
enable_uart=1       # Enable main UART (PL011 on GPIO14/15)
dtoverlay=uart1     # Optional: enable additional UARTs
```
### Ensure `/boot/firmware/config.txt` includes:
```
usercfg.txt
```
## Raspberry Pi OS
```
sudo vi /boot/config.txt
```
### Add the following lines:
```
enable_uart=1
dtoverlay=uart1
```
# 2. Disable Serial Console

## Ubuntu
```
sudo systemctl stop serial-getty@ttyAMA0.service
sudo systemctl disable serial-getty@ttyAMA0.service
```
Edit `/boot/firmware/cmdline.txt` and remove "console=serial0,115200" if present

## Raspberry Pi OS
```
sudo raspi-config
```

# 3. Reboot
```
sudo reboot
```
# 4. Verify UART

## Ubuntu
```
ls -l /dev/ttyAMA0    # Main UART (PL011)
ls -l /dev/ttyS0      # Mini UART
dmesg | grep tty
```
## Raspberry Pi OS
```
ls -l /dev/serial0    # Alias to main UART (PL011)
ls -l /dev/ttyS0      # Mini UART
dmesg | grep tty
```
