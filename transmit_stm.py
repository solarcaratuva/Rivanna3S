import argparse
import platform
import subprocess
import shutil
import os
import sys
import binascii
import serial.tools.list_ports
import time

OS = platform.system()

BLOCK_SIZE = 64
BAUD = 115200

BOARD_MAP = {
    # "power": "cmake_build/NUCLEO_F413ZH/develop/GCC_ARM/PowerBoard/PowerBoard.bin",
    # "telemetry": "cmake_build/POWER_BOARD/develop/GCC_ARM/TelemetryBoard/TelemetryBoard.bin",
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
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "stlink" in port.description.lower() or "st-link" in port.description.lower():
            return port.device
    
    print("ERROR: ST-Link could not be found")
    sys.exit(1)

def wait_for(ser, token: bytes):
    while True:
        data = ser.read(len(token))
        if data == token:
            return
        else:
            print(f"Received unexpected data: {data} - expected: {token}")

def upload_file(port: str, filepath: str, board_id: int):
    with serial.Serial(port, BAUD, timeout=2) as ser:

        print(f"Connected to {port}")

        # --- Send command to select board ---
        print(f"Selecting board {board_id}...")
        cmd = f"{board_id}\n".encode()
        ser.write(cmd)
        

        with open(filepath, "rb") as f:
            block_num = 0

            while True:
                data = f.read(BLOCK_SIZE)
                if not data:
                    print("End of file reached")
                    break
                
                # print("Waiting for REQ...")
                wait_for(ser, b"REQ")

                if len(data) < BLOCK_SIZE:
                    data = data.ljust(BLOCK_SIZE, b"\xFF")

                crc = crc16(data)

                packet = data + crc.to_bytes(2, "big")
                ser.write(packet)

                resp = ser.read(4)

                if resp == b"ACK\n":
                    block_num += 1
                    print(f"Block {block_num} OK")

                elif resp == b"NACK":
                    print("Resending block...")
                    f.seek(-BLOCK_SIZE, 1)

                else:
                    print(f"Unexpected response: {resp}")
                    raise RuntimeError("Unexpected response")

        print("Waiting for DONE...")
        wait_for(ser, b"REQ") #Board sends REQ before DONE
        wait_for(ser, b"DONE")

        print("Upload complete")

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
        "--save",
        type=str,
        default=None,
        help="Save output to file"
    )

    parser.add_argument(
        "--filter",
        type=str,
        default=None,
        help="Filter output"
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
    if args.save:
        args.save = os.path.abspath(args.save)
        process = subprocess.run(f"wslpath -w {args.save}", capture_output=True, shell=True, text=True, check=True)
        path = process.stdout.strip()
        path = path.replace("\\", "\\\\")  # Escape backslashes for Windows command line
        s += f" --save {path}"
    if args.filter:
        s += f" --filter {args.filter}"
    return s

def main() -> None:
    args = get_args()

    #Check OS
    if OS == "Linux" and is_wsl():
        copy_file_to_windows(os.path.abspath(__file__))
        windows_firmware_path = copy_firmware_to_windows(args.filepath)
        args_str = compress_args(args, windows_firmware_path)
        subprocess.run(f"cmd.exe /c start \"\" cmd /k python C:\\\\Windows\\\\Temp\\\\transmit_stm.py {args_str}", capture_output=True, shell=True, check=True)
        return

    port = args.port if args.port else get_correct_port()

    upload_file(
        port=port,
        filepath=args.filepath,
        board_id=BOARD_ID_MAP[args.board]   
    )




if __name__ == "__main__":
    main()