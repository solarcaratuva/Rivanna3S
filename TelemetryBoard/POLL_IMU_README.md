# poll_imu — VectorNav VN200 Polling Module

Polls the VectorNav VN200 IMU over UART and returns parsed INS data in a single function call.

## Files

| File | Location | Purpose |
|------|----------|---------|
| `poll_imu.h` | `TelemetryBoard/Inc/` | `IMUData` struct and `poll_imu()` declaration |
| `poll_imu.cpp` | `TelemetryBoard/Src/` | UART communication, command building, response parsing |

---

## How It Works

```
poll_imu() called
    │
    ├─ 1. build_command()    Translates data type (e.g. "VNINS") into the
    │                        VN200 register-read command: "$VNRRG,58*XX\r\n"
    │
    ├─ 2. UART open          Creates a UART connection on the pins defined
    │                        in pindef.h (USB_TX / USB_RX) at 115200 baud
    │
    ├─ 3. UART write         Sends the command to the VN200
    │
    ├─ 4. UART read          Reads the response (blocking, 500 ms timeout)
    │
    └─ 5. parse_vnins()      Tokenizes the CSV response and extracts:
                             time, yaw, pitch, roll, latitude, longitude, altitude
```

### Key Design Choices

- **No global state** — the UART object is created on the stack each call, so there's no shared mutable state to manage.
- **Extensible** — to add a new data type, add a case in `build_command()` and a corresponding `parse_*()` function.
- **Fail-safe** — returns `-1` on any error (bad pins, timeout, malformed response). Returns `0` on success.

---

## Usage

```cpp
#include "poll_imu.h"

IMUData imu;

// Default data type ("VNINS")
if (poll_imu(NULL, &imu) == 0) {
    // imu.yaw, imu.pitch, imu.roll
    // imu.latitude, imu.longitude, imu.altitude
    // imu.time
}

// Explicit data type
if (poll_imu("VNINS", &imu) == 0) {
    // same fields
}
```

---

## Building

This project cross-compiles for the **STM32H743** (ARM Cortex-M7) using `arm-none-eabi-gcc`. You need the ARM GCC toolchain installed at `/usr/bin/arm-none-eabi-*`.

### First-time setup (configure)

```bash
cd /home/bhyun/Rivanna3S
mkdir -p build && cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

> The `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` flag generates a `compile_commands.json` that fixes the clang/IntelliSense errors in VS Code. After building, symlink it to the project root:
> ```bash
> ln -sf /home/bhyun/Rivanna3S/build/compile_commands.json /home/bhyun/Rivanna3S/
> ```

### Build

```bash
cd /home/bhyun/Rivanna3S/build
cmake --build . --target TelemetryBoard.elf
```

The output binary will be at `build/bin/TelemetryBoard.elf`.

### Build all boards

```bash
cmake --build .
```

---

## Configuration

All config is at the top of `poll_imu.cpp`:

| Define | Default | Purpose |
|--------|---------|---------|
| `IMU_TX_PIN` | `USB_TX` (PC_10) | UART TX pin to VN200 |
| `IMU_RX_PIN` | `USB_RX` (PC_11) | UART RX pin to VN200 |
| `IMU_BAUD` | `115200` | Baud rate |
| `READ_BUF_SIZE` | `256` | Response buffer size (bytes) |
| `READ_TIMEOUT` | `500` | Read timeout (ms) |

Update `IMU_TX_PIN` / `IMU_RX_PIN` if the VN200 is wired to different pins.

---

## VN200 Protocol Reference

The VN200 uses an ASCII serial protocol. This module sends a **register read** command:

```
$VNRRG,58*XX\r\n        →  Read register 58 (INS Solution)
```

The VN200 responds with:

```
$VNINS,<time>,<week>,<status>,<yaw>,<pitch>,<roll>,<lat>,<lon>,<alt>,...*<checksum>
```

`parse_vnins()` extracts fields by index:

| Index | Field | Type |
|-------|-------|------|
| 0 | GPS time of week (s) | `double` |
| 3 | Yaw (°) | `float` |
| 4 | Pitch (°) | `float` |
| 5 | Roll (°) | `float` |
| 6 | Latitude (°) | `double` |
| 7 | Longitude (°) | `double` |
| 8 | Altitude (m) | `double` |
