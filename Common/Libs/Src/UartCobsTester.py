# #!/usr/bin/env python3

# from cobs import cobs

# def strip_trailing_delimiter(encoded: bytes) -> bytes:
#     # Your firmware appends a 0x00 delimiter.
#     # Remove exactly one trailing 00 if present.
#     if len(encoded) > 0 and encoded[-1] == 0x00:
#         return encoded[:-1]
#     return encoded

# def check_case(payload_hex: str, encoded_hex: str):
#     payload = bytes.fromhex(payload_hex)
#     encoded_with_delim = bytes.fromhex(encoded_hex)

#     encoded = strip_trailing_delimiter(encoded_with_delim)

#     # Decode firmware output
#     decoded = cobs.decode(encoded)

#     # Re-encode using Python
#     python_encoded = cobs.encode(payload)

#     print("Payload (input):     ", payload_hex)
#     print("Firmware encoded:    ", encoded.hex())
#     print("Python re-encoded:   ", python_encoded.hex())
#     print("Decoded result:      ", decoded.hex())
#     print()

#     if decoded != payload:
#         print("DECODE MISMATCH")
#         return

#     if python_encoded != encoded:
#         print("Decode OK but encode differs")
#         return

#     print("PASS — Encode and Decode match exactly")

# def rep(byte_hex: str, n: int) -> str:
#     return byte_hex * n

# def seq(start: int, n: int) -> str:
#     return bytes((start + i) & 0xFF for i in range(n)).hex()


# if __name__ == "__main__":

#     test_cases = [
#         # p0
#         {
#             "payload_hex": "",
#             "encoded_hex": "0100"
#         },

#         # p1
#         {
#             "payload_hex": "00",
#             "encoded_hex": "010100"
#         },

#         # p2
#         {
#             "payload_hex": "110022142115",
#             "encoded_hex": "0211052214211500"
#         },

#         # p3
#         {
#             "payload_hex": "01020304050102030405",
#             "encoded_hex": "0b0102030405010203040500"
#         },

#         # p4
#         {
#             "payload_hex": "00000001000200010002000100020001000200",
#             "encoded_hex": "010101020102020201020202010202020102020100"
#         },
#     ]

#     for i, case in enumerate(test_cases):
#         print(f"\n===== Test Case {i} =====")
#         check_case(case["payload_hex"], case["encoded_hex"])