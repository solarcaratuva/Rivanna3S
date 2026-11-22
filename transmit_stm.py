import argparse
import platform
import subprocess
import shutil
import os
import sys
import binascii
import time
from pathlib import Path

OS = platform.system()

BLOCK_SIZE = 64
BAUD = 115200
# Keep these protocol limits aligned with FirmwareUploader's FW_* constants.
MAX_FIRMWARE_SIZE = 4 * 1024 * 1024
CRC_INIT = 0xFFFF
MAX_RETRIES = 3  # Retries after the initial block attempt.
SETUP_TIMEOUT = 20  # Includes remote inactive-bank erase; block waits stay short.
VERIFY_TIMEOUT = 12  # Full-bank CRC can take seconds on a G4 debug build.

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
    return binascii.crc_hqx(data, CRC_INIT)

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

        # fdcanusb also uses ST's VID; it speaks adapter commands, not our UART protocol.
        if "fdcanusb" in " ".join((desc, mfg, prod, hwid)):
            continue

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
    pending = bytearray()
    while time.monotonic() < deadline:
        fragment = ser.readline(256)
        if not fragment:
            continue
        pending.extend(fragment)
        if not fragment.endswith(b"\n"):
            continue
        line = bytes(pending).strip()
        pending.clear()
        if line.startswith(b"FW "):
            if line == b"FW FAIL" or line.startswith(b"FW FAIL "):
                reason = line[7:].decode("ascii", errors="replace").strip()
                raise RuntimeError(reason or "Board rejected or failed the transfer")
            status = {
                b"FW MODE MOCK": "Mock receiver: verifies data only; no firmware will be flashed.",
                b"FW MODE CAN": "CAN update: writing the target's inactive flash bank; no boot switch or reset.",
                b"FW SETUP_ACK": "Target acknowledged setup; erasing its inactive bank...",
                b"FW READY": "Target ready for firmware data.",
                b"FW VERIFY": "Verifying firmware in target flash...",
            }.get(line)
            if status:
                print(status)
                continue
            return line
        # The same UART continues to carry ordinary log lines.
    raise TimeoutError("Timed out waiting for the board; restart the upload")


def transmit_stream(ser, firmware, board_id):
    # One size/CRC header replaces timeout-as-EOF. Keep the existing 64+2-byte
    # block format and retry only explicit NACKs; a lost ACK aborts unambiguously.
    size, image_crc = 0, CRC_INIT
    start = firmware.tell()
    while True:
        chunk = firmware.read(4096)
        if not chunk:
            break
        size += len(chunk)
        image_crc = binascii.crc_hqx(chunk, image_crc)
    firmware.seek(start)
    if board_id not in BOARD_ID_MAP.values() or not 0 < size <= MAX_FIRMWARE_SIZE:
        raise ValueError("Invalid board or file size (transport accepts 1 byte through 4 MiB; target capacity may be smaller)")

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
        for attempt in range(MAX_RETRIES + 1):
            timeout = SETUP_TIMEOUT if blocks == 0 and attempt == 0 else 3
            if read_response(ser, timeout=timeout) != b"FW REQ":
                raise RuntimeError("Expected a request for the next block")
            if ser.write(packet) != len(packet):
                raise OSError("Short serial write")
            response = read_response(ser)
            if response == b"FW ACK":
                break
            if response != b"FW NACK" or attempt == MAX_RETRIES:
                raise RuntimeError("Block rejected or acknowledgement missing; restart the upload")
            # Keep this exact packet, including a short final block, for retry.
        blocks += 1

    response = read_response(ser, timeout=VERIFY_TIMEOUT).split()
    if (len(response) != 4 or response[:2] != [b"FW", b"DONE"] or
            int(response[2]) != size or int(response[3], 16) != image_crc):
        raise RuntimeError("Consumer's final byte count or CRC does not match the file")
    return size, blocks


def upload_file(port: str, filepath: str, board_id: int, baud=BAUD):
    import serial
    try:
        with serial.Serial(port, baud, timeout=0.2, write_timeout=1) as ser, open(filepath, "rb") as firmware:
            print(f"Connected to {port}; transmitting firmware for board {board_id}...")
            size, blocks = transmit_stream(ser, firmware, board_id)
            print(f"Transfer verified: {size} bytes in {blocks} blocks.")
    except serial.SerialException as err:
        err_msg = str(err).lower()
        if "permission denied" in err_msg or "errno 13" in err_msg:
            print(f"\nERROR: Permission denied opening serial port '{port}'.\n"
                  f"On Linux, add your user to the 'dialout' group to access serial ports without root:\n"
                  f"  sudo usermod -aG dialout $USER\n"
                  f"Then log out and log back in, or run with appropriate permissions.\n", file=sys.stderr)
        raise

def get_args():
    parser = argparse.ArgumentParser(
        description="Transmit firmware over ST-Link UART or a mjbots fdcanusb adapter"
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
        default=BAUD,
        help=f"Baud rate (default: {BAUD})"
    )

    parser.add_argument(
        "--native",
        action="store_true",
        help="Force native Linux serial communication even in WSL"
    )
    parser.add_argument("--can", action="store_true", help="Use a mjbots fdcanusb adapter directly (250 kbit/s CAN FD)")
    parser.add_argument("--activate", action="store_true", help="After CAN verification, switch banks and confirm boot (G474)")
    parser.add_argument("--build", action="store_true", help="Build the selected board before transmitting")
    parser.add_argument("--build-dir", help="Use an existing local CMake build directory instead of compile.py/Docker")
    parser.add_argument("--nucleo", action="store_true", help="Build TopDistBoard using the NUCLEO-G474RE hardware profile (requires --build)")
    parser.add_argument("--source-board", choices=BOARD_MAP.keys(), help="CAN sender identity; must differ from the target (default: motor, or bottomdist for motor)")

    args = parser.parse_args()
    if args.nucleo:
        if args.board != "topdist":
            parser.error("The Nucleo application profile is supported on topdist")
        if not args.build:
            parser.error("--nucleo selects build settings and requires --build")
    if args.activate and not args.can:
        parser.error("--activate requires --can; UART activation is not implemented")
    if args.source_board and not args.can:
        parser.error("--source-board requires --can")
    args.source_board = args.source_board or ("bottomdist" if args.board == "motor" else "motor")
    if args.can and args.source_board == args.board:
        parser.error("CAN sender and target identities must differ")
    if args.build and args.file:
        parser.error("Use --build with the board's generated file, or --file for a prebuilt image")
    if args.build_dir and not args.build:
        parser.error("--build-dir requires --build")
    if args.baud <= 0:
        parser.error("--baud must be positive")
    return args


def prepare_firmware(args):
    root = Path(__file__).resolve().parent
    filename = Path(BOARD_MAP[args.board]).name
    build_dir = Path(args.build_dir or "build")
    if not build_dir.is_absolute():
        build_dir = root / build_dir
    if args.build:
        target = Path(filename).with_suffix(".elf").name
        if args.build_dir:
            cache = build_dir / "CMakeCache.txt"
            if not cache.is_file():
                raise ValueError(f"Configure the local CMake build directory first: {build_dir}")
            config = cache.read_text()
            if "CMAKE_PROJECT_stm32h743_freertos_INCLUDE:" in config:
                raise ValueError("This build uses an external project hook. Configure a fresh build directory using repository sources.")
            command = ["cmake", "-S", str(root), "-B", str(build_dir),
                       f"-DNUCLEO_G474RE={'ON' if args.nucleo else 'OFF'}"]
            if args.can:
                command.extend(["-DCAN_FIRMWARE_UPDATE=ON", "-DUART_UPLOAD_MOCK=OFF"])
            if args.nucleo:
                command.append("-DBOARD=STM32G474RET6")
            subprocess.run(command, check=True)
            subprocess.run(["cmake", "--build", str(build_dir), "--target", target, "-j", "4"], check=True)
        else:
            command = [sys.executable, str(root / "compile.py")]
            if args.can:
                command.append("--can-update")
            if args.nucleo:
                command.append("--nucleo")
            subprocess.run(command + ["--", "--target", target, "-j", "4"], cwd=root, check=True)
    args.filepath = str(Path(args.file).resolve() if args.file else build_dir / "bin" / filename)
    if not os.path.isfile(args.filepath):
        raise ValueError(f"Firmware file not found: {args.filepath}. Build it first or specify --file.")


def copy_file_to_windows(wsl_path: str) -> None:
    path = "/" + os.path.join("mnt", "c", "Windows", "Temp", "transmit_stm.py")
    shutil.copy(wsl_path, path)
    module = Path(wsl_path).with_name("can_upload.py")
    shutil.copy(module, Path(path).with_name(module.name))

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
    if args.baud != BAUD:
        s += f" --baud {args.baud}"
    if args.can:
        s += f" --can --source-board {args.source_board}"
    if args.activate:
        s += " --activate"
    return s

def main() -> None:
    args = get_args()
    prepare_firmware(args)
    print(f"Firmware: {args.filepath}", flush=True)

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

    if args.can:
        from can_upload import upload_file as can_upload_file
        can_upload_file(args.port, args.filepath, BOARD_ID_MAP[args.board],
                        BOARD_ID_MAP[args.source_board], args.baud, args.activate)
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
    except KeyboardInterrupt:
        print("\nUpload cancelled.", file=sys.stderr)
        sys.exit(130)
    except (OSError, RuntimeError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Upload failed: {error}", file=sys.stderr)
        sys.exit(1)
