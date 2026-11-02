"""Host-side #129 protocol checks; hardware UART/RTOS behavior needs bench tests."""
import binascii
import io
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import transmit_stm as uploader


class MockSerial:
    def __init__(self, reject_offset=None, fail_ack=False, bad_final=False):
        self.lines = []
        self.data = bytearray()
        self.packets = []
        self.reject_offset = reject_offset
        self.fail_ack = fail_ack
        self.bad_final = bad_final

    def reset_input_buffer(self):
        self.lines.clear()

    def write(self, data):
        if data.startswith(b'UPLOAD '):
            _, self.board, size, _ = data.split()
            self.size = int(size)
            self.lines.extend([b'INFO: ordinary log\n', b'FW REQ\n'])
        else:
            assert len(data) == 66
            assert uploader.crc16(data[:64]) == int.from_bytes(data[64:], 'big')
            self.packets.append(data)
            if len(self.data) == self.reject_offset:
                self.reject_offset = None
                self.lines.extend([b'FW NACK\n', b'FW REQ\n'])
            elif self.fail_ack:
                self.lines.append(None)
            else:
                self.data.extend(data[:min(64, self.size - len(self.data))])
                self.lines.extend([b'INFO: another log\n', b'FW ACK\n'])
                if len(self.data) < self.size:
                    self.lines.append(b'FW REQ\n')
                else:
                    crc = binascii.crc_hqx(self.data, 0xffff) ^ self.bad_final
                    self.lines.append(f'FW DONE {len(self.data)} {crc:04x}\n'.encode())
        return len(data)

    def readline(self, size):
        line = self.lines.pop(0)
        if line is None:
            raise TimeoutError('missing ACK')
        return line


class UploadTests(unittest.TestCase):
    def test_sizes_and_logging(self):
        for size in (1, 64, 65, 256 * 1024 + 1):
            with self.subTest(size=size):
                data = bytes(i % 251 for i in range(size))
                serial = MockSerial()
                self.assertEqual(uploader.transmit_stream(serial, io.BytesIO(data), 4),
                                 (size, (size + 63) // 64))
                self.assertEqual(serial.data, data)
                self.assertEqual(serial.board, b'4')

    def test_retry_preserves_partial_final_packet(self):
        serial = MockSerial(reject_offset=64)
        uploader.transmit_stream(serial, io.BytesIO(b'x' * 65), 1)
        self.assertEqual(serial.packets[1], serial.packets[2])
        self.assertEqual(serial.data, b'x' * 65)

    def test_missing_ack_does_not_resend(self):
        serial = MockSerial(fail_ack=True)
        with self.assertRaises(TimeoutError):
            uploader.transmit_stream(serial, io.BytesIO(b'x'), 1)
        self.assertEqual(len(serial.packets), 1)

    def test_bad_final_crc_fails(self):
        with self.assertRaises(RuntimeError):
            uploader.transmit_stream(MockSerial(bad_final=True), io.BytesIO(b'x'), 1)

    def test_timeout_is_bounded(self):
        with patch.object(uploader.time, 'monotonic', side_effect=[0, 4]):
            with self.assertRaises(TimeoutError):
                uploader.read_response(MockSerial())

    def test_empty_file_rejected_before_write(self):
        serial = MockSerial()
        with self.assertRaises(ValueError):
            uploader.transmit_stream(serial, io.BytesIO(), 1)
        self.assertEqual(serial.lines, [])


if __name__ == '__main__':
    unittest.main()
