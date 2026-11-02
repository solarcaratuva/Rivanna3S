# #129: laptop-to-board bench test

Scope: [#129](https://github.com/solarcaratuva/Rivanna3S/issues/129), with
[Ethan's recovery guidance](https://github.com/solarcaratuva/Rivanna3S/issues/234#issuecomment-6562480441).
Use the logging UART through ST-Link, accept a destination ID, request blocks,
check each block's CRC, and mock the board-to-board consumer. No CAN update,
flash erase/program, bank switching, board-image validation (#131), radio work,
or progress UI (#132) is implemented by this test mode.

## Build and connection

Build `TopDistBoard.elf` for `STM32G474RET6` with `UART_UPLOAD_MOCK=ON`.
The existing Docker helper accepts `python3 compile.py --mock -- --target TopDistBoard.elf`.
Its container must be mounted to this checkout. Alternatively configure CMake
with the installed ARM toolchain and `-DUART_UPLOAD_MOCK=ON`, then build the target.
MotorBoard also supports this flag. Omitting `--mock` explicitly disables it in
`compile.py`; ordinary application behavior remains the default.

For G474 TopDistBoard, the shared logging/upload UART uses TX PC12 and RX PD2
at 115200 baud in mock mode. Confirm these connect to the ST-Link virtual COM
port; USB enumeration alone does not verify the wiring. Normal TopDistBoard
application tasks still run, so use the intended bench hardware configuration.
Install the resulting test firmware through the normal ST-Link programming
procedure before attempting a transfer. The Python command only sends test data;
it does not install firmware on the MCU.

```sh
python3 transmit_stm.py topdist --file /path/to/payload.bin --port /dev/ttyACM0
python3 -m unittest discover -s tests -v
```

## Protocol and acceptance

The command is `UPLOAD <destination-id> <byte-count> <crc16-hex>\n`.
The mock accepts IDs 0–7 without routing to another board. Each `FW REQ` requests
64 bytes padded with FF, followed by a big-endian CRC-16 (binascii.crc_hqx,
seed FFFF) over those 64 bytes. `FW ACK` accepts a block; `FW NACK` permits a
bounded retry of the same block. Missing acknowledgements abort the attempt.
Ordinary log lines are ignored by the host.

An eight-block queue feeds a mock consumer that counts and checksums bytes;
it does not retain the whole file. `FW DONE <byte-count> <crc16-hex>` verifies
consumption of the original bytes, excluding padding. The size/image CRC header
provides explicit completion instead of treating a timeout as success.
The 4 MiB limit is a mock transport limit, not the MCU's flash capacity.

Before declaring #129 complete, exercise on hardware:

- 1-, 64-, and 65-byte payloads, then a payload larger than MCU RAM.
- A corrupted block followed by a valid retry, including the partial last block.
- A stalled/disconnected sender: failure, never success; then repeat an upload
  after allowing two seconds for timeout/recovery.
- Consecutive uploads and ordinary logging on the same UART.

Host tests cover sender behavior only. Firmware compilation does not prove
UART timing, task/heap availability, or wiring. Keep the existing 32 KiB RTOS
heap until runtime evidence warrants a targeted adjustment. Global I2C timeout
changes are outside this issue. Flash work remains gated on Ethan's G474/DBANK
checks and belongs to the later board-to-board work.

## Recorded hardware results — November 2, 2026

With the reviewed G474 TopDistBoard mock image programmed and verified through
STLINK-V3MODS, the user confirmed the following results on `/dev/ttyACM0`:

- 1, 64, 65, and 1 bytes consecutively: verified in 1, 1, 2, and 1 blocks.
- 262,145 bytes: verified in 4,097 blocks, followed by a successful 65-byte upload.
- Deliberately corrupted CRC: NACK, then ACK and matching DONE after a valid retry
  of the one-byte padded final block.
- Sender closed after the first block of a 65-byte transfer: a subsequent
  65-byte upload succeeded without resetting the MCU.

Earlier timeouts occurred with a different installed firmware image. The results
above apply to the reviewed image; they do not establish the cause of those
timeouts. Recovery was observed after interruption; the disconnected sender did
not capture the board's timeout response. Concurrent ordinary logging under load
was not separately exercised. No CAN delivery or flash-update behavior was tested.
