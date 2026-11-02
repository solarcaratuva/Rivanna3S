import argparse
import platform
import subprocess
import shutil
import os
import sys
import binascii
import time

OS = platform.system()

BLOCK_SIZE = 64
BAUD = 115200

BOARD_MAP = {
    "bottomdist": "build/bin/BottomDistBoard.bin",
    "motor": "build/bin/MotorBoard.bin",
    "relay": "build/bin/RelayBoard.bin",
    "telemetry": "build/bin/TelemetryBoard.bin",
    "topdist": "build/bin/TopDistBoard.bin",
    "wheel": "build/bin/WheelBoard.bin",
    "bms": "build/bin/BMSBoard.bin",
    "motorctrl": "build/bin/MotorCtrlBoard.bin",
}

BOARD_ID_MAP = {
    "bottomdist": 0,
    "motor":      1,
    "relay":      2,
    "telemetry":  3,
    "topdist":    4,
    "wheel":      5,
    "bms":        6,
    "motorctrl":   7,
}

def crc16(data: bytes) -> int:
    return binascii.crc_hqx(data, 0xFFFF)

def get_correct_port() -> str:
    import glob
    import serial.tools.list_ports

    # 1. On Linux, check /dev/serial/by-id/ symlinks created by udev
    if os.path.isdir("/dev/serial/by-id"):
        for path in sorted(glob.glob("/dev/serial/by-id/*")):
            name = os.path.basename(path).lower()
            if any(k in name for k in ["stlink", "st-link", "stmicroelectronics", "stm32"]):
                real_device = os.path.realpath(path)
                print(f"Detected ST-Link via /dev/serial/by-id: {os.path.basename(path)} -> {real_device}")
                return real_device

    # 2. Search pySerial comports with robust matching (VID, manufacturer, product, description)
    ports = serial.tools.list_ports.comports()
    st_ports = []
    generic_usb_ports = []

    for port in ports:
        desc = (port.description or "").lower()
        mfg = (port.manufacturer or "").lower()
        prod = (port.product or "").lower()
        hwid = (port.hwid or "").lower()
        vid = getattr(port, "vid", None)

        # STMicroelectronics Vendor ID is 0x0483 (1155)
        is_st = (vid == 0x0483 or
                 any(k in desc or k in mfg or k in prod or k in hwid
                     for k in ["stlink", "st-link", "stmicroelectronics", "stm32"]))

        if is_st:
            st_ports.append(port.device)
        elif port.device.startswith(("/dev/ttyACM", "/dev/ttyUSB", "COM")):
            generic_usb_ports.append(port.device)

    if st_ports:
        return st_ports[0]

    # 3. Fallback: if exactly one USB CDC-ACM / USB serial port is present, use it
    if len(generic_usb_ports) == 1:
        print(f"No explicit ST-Link label found, using sole USB serial device: {generic_usb_ports[0]}")
        return generic_usb_ports[0]

    # 4. Error reporting with candidate listing
    print("ERROR: ST-Link could not be auto-detected.", file=sys.stderr)
    active_ports = [p.device for p in ports if not p.device.startswith("/dev/ttyS")]
    if active_ports:
        print(f"Available serial ports: {', '.join(active_ports)}", file=sys.stderr)
        print("Specify the port manually with -p / --port (e.g. -p /dev/ttyACM0)", file=sys.stderr)
    else:
        print("No USB serial devices detected. Ensure the board's ST-Link USB is plugged in.", file=sys.stderr)
    sys.exit(1)

def read_response(ser, timeout=3):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        line = ser.readline(256).strip()
        if line.startswith(b"FW "):
            if line == b"FW FAIL":
                raise RuntimeError("Board rejected or failed the transfer")
            return line
        # The same UART continues to carry ordinary log lines.
    raise TimeoutError("Timed out waiting for the board; restart the upload")


def transmit_stream(ser, firmware, board_id):
    # One size/CRC header replaces timeout-as-EOF. Keep the existing 64+2-byte
    # block format and retry only explicit NACKs; a lost ACK aborts unambiguously.
    size, image_crc = 0, 0xFFFF
    start = firmware.tell()
    while True:
        chunk = firmware.read(4096)
        if not chunk:
            break
        size += len(chunk)
        image_crc = binascii.crc_hqx(chunk, image_crc)
    firmware.seek(start)
    if board_id not in BOARD_ID_MAP.values() or not 0 < size <= 4 * 1024 * 1024:
        raise ValueError("Invalid board or file size (mock accepts 1 byte through 4 MiB)")

    command = f"UPLOAD {board_id} {size} {image_crc:04x}\n".encode()
    ser.reset_input_buffer()
    if ser.write(command) != len(command):
        raise OSError("Short serial write")
    blocks = 0
    while True:
        data = firmware.read(BLOCK_SIZE)
        if not data:
            break
        padded = data.ljust(BLOCK_SIZE, b"\xff")
        packet = padded + crc16(padded).to_bytes(2, "big")
        for attempt in range(4):
            if read_response(ser) != b"FW REQ":
                raise RuntimeError("Expected a request for the next block")
            if ser.write(packet) != len(packet):
                raise OSError("Short serial write")
            response = read_response(ser)
            if response == b"FW ACK":
                break
            if response != b"FW NACK" or attempt == 3:
                raise RuntimeError("Block rejected or acknowledgement missing; restart the upload")
            # Keep this exact packet, including a short final block, for retry.
        blocks += 1

    response = read_response(ser).split()
    if (len(response) != 4 or response[:2] != [b"FW", b"DONE"] or
            int(response[2]) != size or int(response[3], 16) != image_crc):
        raise RuntimeError("Consumer's final byte count or CRC does not match the file")
    return size, blocks


def upload_file(port: str, filepath: str, board_id: int, baud=BAUD):
    import serial
    try:
        with serial.Serial(port, baud, timeout=0.2, write_timeout=1) as ser, open(filepath, "rb") as firmware:
            print(f"Connected to {port}; uploading to mock board {board_id}...")
            size, blocks = transmit_stream(ser, firmware, board_id)
            print(f"Mock consumer verified {size} bytes in {blocks} blocks. No firmware was flashed.")
    except serial.SerialException as err:
        err_msg = str(err).lower()
        if "permission denied" in err_msg or "errno 13" in err_msg:
            print(f"\nERROR: Permission denied opening serial port '{port}'.\n"
                  f"On Linux, add your user to the 'dialout' group to access serial ports without root:\n"
                  f"  sudo usermod -aG dialout $USER\n"
                  f"Then log out and log back in, or run with appropriate permissions.\n", file=sys.stderr)
        raise

def get_args():
    import argparse
    import os

    parser = argparse.ArgumentParser(
        description="Upload firmware to STM board over ST-Link VCP"
    )

    parser.add_argument(
        "board",
        type=str,
        choices=BOARD_MAP.keys(),
        help="Target board name"
    )

    parser.add_argument(
        "-f", "--file",
        type=str,
        default=None,
        help="Override firmware file path"
    )

    parser.add_argument(
        "-p", "--port",
        type=str,
        default=None,
        help="Serial port (auto-detected if not provided)"
    )

    parser.add_argument(
        "--baud",
        type=int,
        default=115200,
        help="Baud rate (default: 115200)"
    )

    parser.add_argument(
        "--native",
        action="store_true",
        help="Force native Linux serial communication even in WSL"
    )

    args = parser.parse_args()

    # --- Resolve firmware file ---

    if args.file:
        filepath = args.file
    else:
        filepath = BOARD_MAP[args.board]

    if not os.path.isfile(filepath):
        parser.error(f"Firmware file not found: {filepath}")

    args.filepath = filepath

    return args


def copy_file_to_windows(wsl_path: str) -> None:
    path = "/" + os.path.join("mnt", "c", "Windows", "Temp", "transmit_stm.py")
    shutil.copy(wsl_path, path)

def copy_firmware_to_windows(firmware_path: str) -> str:
    filename = os.path.basename(firmware_path)
    dest = "/" + os.path.join("mnt", "c", "Windows", "Temp", filename)
    shutil.copy(firmware_path, dest)
    return f"C:\\Windows\\Temp\\{filename}"

def is_wsl() -> bool:
    try:
        if "microsoft" in platform.uname().release.lower(): # Check uname release for "microsoft" (common in WSL1/WSL2)
            return True
        if "WSL_INTEROP" in os.environ:
            return True
        with open("/proc/version", "r") as f:  # Check /proc/version for "Microsoft"
            if "microsoft" in f.read().lower():
                return True
    except Exception:
        pass
    return False

def compress_args(args, windows_firmware_path: str = None):
    s = f" {args.board}"
    firmware_path = windows_firmware_path if windows_firmware_path else args.file
    if firmware_path:
        s += f" --file \"{firmware_path}\""
    if args.port:
        s += f" --port {args.port}"
    if args.baud != 115200:
        s += f" --baud {args.baud}"
    return s

def main() -> None:
    args = get_args()

    # Check for WSL bridging to Windows
    # If running on native Linux, or if the user explicitly provided a Linux device (e.g. /dev/ttyACM0)
    # or passed --native, run natively without launching Windows cmd.exe.
    is_linux_port = bool(args.port and args.port.startswith("/dev/"))
    if OS == "Linux" and is_wsl() and not args.native and not is_linux_port:
        copy_file_to_windows(os.path.abspath(__file__))
        windows_firmware_path = copy_firmware_to_windows(args.filepath)
        args_str = compress_args(args, windows_firmware_path)
        subprocess.run(f"cmd.exe /c start \"\" cmd /k python C:\\\\Windows\\\\Temp\\\\transmit_stm.py {args_str}", capture_output=True, shell=True, check=True)
        return

    port = args.port if args.port else get_correct_port()

    upload_file(
        port=port,
        filepath=args.filepath,
        board_id=BOARD_ID_MAP[args.board],
        baud=args.baud
    )




if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Upload failed: {error}", file=sys.stderr)
        sys.exit(1)
