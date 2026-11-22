"""Direct mjbots fdcanusb transport for the FirmwareUpdate CAN protocol."""

import binascii
from collections import deque
import glob
import os
from pathlib import Path
import secrets
import struct
import time

import serial
import serial.tools.list_ports

CONTROL_ID = 0x76C
DATA_ID = 0x7D0
BLOCK_SIZE = 64
FLAGS = {1: 0x10, 2: 0x20, 3: 0x40, 6: 0x80, 7: 0x80}
ERRORS = {
    1: "busy", 2: "invalid destination", 3: "unsupported flash layout",
    4: "image too large", 5: "erase failed", 6: "write failed",
    7: "protocol error", 8: "block CRC mismatch", 9: "timeout",
    10: "CAN transmit failed", 11: "receive queue overflow",
    12: "final CRC mismatch", 13: "cancelled", 14: "activation failed",
}


def find_port():
    candidates = {}
    for path in sorted(glob.glob("/dev/serial/by-id/*")):
        if "fdcanusb" in os.path.basename(path).lower():
            candidates[os.path.realpath(path)] = path
    for port in serial.tools.list_ports.comports():
        description = " ".join(str(value or "") for value in
                               (port.description, port.product, port.manufacturer))
        if "fdcanusb" in description.lower():
            candidates.setdefault(os.path.realpath(port.device), port.device)
    if len(candidates) != 1:
        raise RuntimeError("Expected one mjbots fdcanusb adapter; specify --port. "
                           f"Detected: {', '.join(candidates.values()) or 'none'}")
    return next(iter(candidates.values()))


class CanUpload:
    def __init__(self, port, target, source, baud=115200):
        options = {"exclusive": True} if os.name == "posix" else {}
        self.serial = serial.Serial(port, baud, timeout=0.1, write_timeout=1, **options)
        self.target = target
        self.source = source
        self.session = secrets.randbelow(255) + 1
        self.frames = deque(maxlen=64)
        self.pending = bytearray()

    def close(self):
        self.serial.close()

    def line(self, deadline):
        while time.monotonic() < deadline:
            fragment = self.serial.readline(512)
            self.pending.extend(fragment)
            if len(self.pending) > 2048:
                raise RuntimeError("Oversized response from CAN adapter")
            if self.pending.endswith(b"\n"):
                line = self.pending.decode("ascii", errors="replace").strip()
                self.pending.clear()
                return line
        raise TimeoutError("Timed out waiting for CAN adapter or target response")

    def dispatch(self, line):
        fields = line.split()
        if fields and fields[0] == "rcv":
            if len(fields) < 3:
                raise RuntimeError(f"Malformed adapter response: {line}")
            identifier, data = int(fields[1], 16), bytes.fromhex(fields[2])
            if (identifier == CONTROL_ID and len(data) == 12 and
                data[0] & 15 == self.target and data[6] == self.target and
                data[7] == self.session and data[5] >> 4 == 1
            ):
                if len(self.frames) == self.frames.maxlen:
                    raise RuntimeError("CAN response queue overflow")
                self.frames.append((identifier, data))
            return True
        if line.startswith("ERR"):
            raise RuntimeError(f"CAN adapter: {line}")
        return False

    def command(self, text):
        packet = (text + "\n").encode("ascii")
        if self.serial.write(packet) != len(packet):
            raise OSError("Short write to CAN adapter")
        deadline = time.monotonic() + 2
        response = []
        while True:
            line = self.line(deadline)
            if self.dispatch(line):
                continue
            if line == "OK":
                return response
            if line:
                response.append(line)

    def configure(self):
        # Runtime settings only: never write the adapter's persistent config.
        settings = {}
        for line in self.command("conf enumerate"):
            parts = line.split()
            if len(parts) == 2:
                settings[parts[0]] = parts[1]
        required = {"can.bitrate": "250000", "can.fdcan_frame": "1",
                    "can.bitrate_switch": "0", "can.automatic_retransmission": "0",
                    "can.bus_monitor": "0", "can.restricted_mode": "0"}
        if not all(key in settings for key in required):
            raise RuntimeError("Adapter firmware lacks the required CAN FD settings")
        self.command("can off")
        for key, value in required.items():
            if settings[key] != value:
                self.command(f"conf set {key} {value}")
        self.command("can on")
        self.frames.clear()

    def send(self, identifier, data):
        self.command(f"can std {identifier:X} {data.hex()} Fbr")

    def control(self, operation, value=0, checksum=0):
        data = (bytes([self.target | FLAGS.get(operation, 0)]) +
                struct.pack("<I", checksum) +
                bytes([0x10 | operation, self.source, self.session]) +
                struct.pack("<I", value))
        self.send(CONTROL_ID, data)

    def wait_for(self, operation, timeout, value=None):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if not self.frames:
                try:
                    self.dispatch(self.line(deadline))
                except TimeoutError as error:
                    raise TimeoutError(f"Timed out waiting for target operation {operation}") from error
                continue
            identifier, data = self.frames.popleft()
            if identifier != CONTROL_ID:
                continue
            received = data[5] & 15
            actual_value = struct.unpack_from("<I", data, 8)[0]
            checksum = struct.unpack_from("<I", data, 1)[0]
            if received in (8, 9):
                raise RuntimeError(f"Target rejected upload: {ERRORS.get(actual_value, actual_value)}")
            if received != operation or data[0] & 0xF0 != FLAGS.get(operation, 0):
                raise RuntimeError(f"Unexpected target operation {received}; expected {operation}")
            if value is not None and actual_value != value:
                raise RuntimeError(f"Target reported {actual_value}; expected {value}")
            return actual_value, checksum
        raise TimeoutError(f"Timed out waiting for target operation {operation}")

    def boot_bank(self):
        self.control(12)
        bank, _ = self.wait_for(13, 2)
        if bank not in (1, 2):
            raise RuntimeError("Activation is supported only by the G474 dual-bank firmware")
        return bank

    def upload(self, image, activate=False):
        size = len(image)
        checksum = binascii.crc_hqx(image, 0xFFFF)
        bank = None
        if activate:
            try:
                bank = self.boot_bank()
            except TimeoutError as error:
                raise RuntimeError("Target does not answer boot queries. --activate requires the "
                                   "activation-capable firmware; no upload started.") from error
        started = False
        activating = False
        try:
            print(f"CAN target {self.target}, sender {self.source}, session {self.session}: "
                  f"{size} bytes, CRC16 {checksum:04X}", flush=True)
            started = True
            self.control(1, size, checksum)
            self.wait_for(2, 2)
            print("Target acknowledged setup; erasing inactive bank...", flush=True)
            capacity, _ = self.wait_for(3, 20)
            if size > capacity:
                raise RuntimeError(f"Image exceeds target capacity ({capacity} bytes)")
            for offset in range(0, size, BLOCK_SIZE):
                data = image[offset:offset + BLOCK_SIZE].ljust(BLOCK_SIZE, b"\xff")
                self.control(4, offset, binascii.crc_hqx(data, 0xFFFF))
                self.send(DATA_ID, data)
                self.wait_for(5, 2, min(offset + BLOCK_SIZE, size))
                if (offset // BLOCK_SIZE + 1) % 128 == 0:
                    print(f"Written {min(offset + BLOCK_SIZE, size)}/{size} bytes", flush=True)
            self.control(6, size, checksum)
            _, actual_crc = self.wait_for(7, 12, size)
            if actual_crc != checksum:
                raise RuntimeError("Target flash CRC does not match the firmware file")
            print(f"Target flash verified: {size} bytes, CRC16 {checksum:04X}.", flush=True)
            if not activate:
                print("Image remains in the inactive bank; it has not been activated.")
                return
            # Once activation is requested, a reset may already be in progress.
            activating = True
            self.control(10, size, checksum)
            _, actual_crc = self.wait_for(11, 3, size)
            if actual_crc != checksum:
                raise RuntimeError("Activation acknowledgement CRC mismatch")
            print("Activation acknowledged; waiting for the new image to boot...", flush=True)
            time.sleep(0.5)
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline:
                try:
                    new_bank = self.boot_bank()
                    if new_bank == 3 - bank:
                        print(f"Boot confirmed over CAN: physical Bank {new_bank}.", flush=True)
                        return
                except TimeoutError:
                    pass
                time.sleep(0.1)
            raise RuntimeError("Flash was verified and activation acknowledged, but boot from "
                               f"Bank {3 - bank} was not confirmed. The new image must support boot queries.")
        except BaseException:
            if started and not activating:
                try:
                    self.control(8)
                except (OSError, RuntimeError, ValueError):
                    pass
            raise


def upload_file(port, filepath, target, source, baud=115200, activate=False):
    image = Path(filepath).read_bytes()
    if not 0 < len(image) <= 4 * 1024 * 1024:
        raise ValueError("Firmware must contain 1 byte through 4 MiB")
    if activate:
        if len(image) < 8:
            raise ValueError("Firmware has no vector table")
        stack, reset = struct.unpack_from("<II", image)
        if (stack % 8 or not 0x20000000 < stack <= 0x20020000 or
                not reset & 1 or not 0x08000000 <= reset - 1 < 0x08000000 + len(image)):
            raise ValueError("Firmware does not have a valid G474 application vector table")
    port = port or find_port()
    print(f"Connected through {port}; CAN FD at 250000 bit/s, no BRS.", flush=True)
    connection = CanUpload(port, target, source, baud)
    try:
        connection.configure()
        connection.upload(image, activate)
    finally:
        connection.close()
