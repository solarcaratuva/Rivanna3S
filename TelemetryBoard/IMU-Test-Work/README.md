# VN200 IMU Test

Single-poll test for the VectorNav VN200 over `/dev/ttyUSB0`.

## Connect IMU to WSL if you are on Windows and connecting through USB
Note: skip if you are on non-windows system
```bash
usbipd list
usbipd attach --busid <busid>
```
Don't forget to detach it after (```usbipd detach --busid <busid>```)

## Quick Start

```bash
chmod +x run.sh
./run.sh
```

This builds the project and runs the test in one step. If you get a permission error on the serial port:

```bash
sudo chmod 666 /dev/ttyUSB0
```
Or, check the port name by running `ls /dev/tty*` and see which one is the IMU.

## What It Does

`main.cpp` calls `sensor.pollOnce(data)` which:

1. Connects to the VN200 at 115200 baud on `/dev/ttyUSB0`
2. Reads serial lines until a valid `$VNINS` message is received (2s timeout)
3. Parses and prints: yaw, pitch, roll, latitude, longitude, altitude
4. Exits

The old continuous polling loop is preserved as a comment in `main.cpp`.

## Files

| File | Purpose |
|------|---------|
| `vn200_linux.h` | `VN200` class and `VNData` struct |
| `vn200_linux.cpp` | Serial connect, read, parse, and `pollOnce()` |
| `main.cpp` | Entry point — single poll call |
| `run.sh` | Build + run script |

## Manual Build

```bash
mkdir -p build && cd build
cmake ..
make
./vn_test
```
