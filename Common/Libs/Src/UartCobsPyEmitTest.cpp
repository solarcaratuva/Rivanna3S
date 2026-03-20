// #include <cstdint>
// #include <cstddef>
// #include <cstdio>
// #include <cstring>   // NEW for memcmp
// #include "UartCobs.h"
// #include "log.h"

// static void bytes_to_hex(const uint8_t* in, size_t n, char* out, size_t out_cap) {
//     static const char* HEX = "0123456789abcdef";
//     size_t j = 0;
//     for (size_t i = 0; i < n && (j + 2) < out_cap; i++) {
//         out[j++] = HEX[(in[i] >> 4) & 0xF];
//         out[j++] = HEX[in[i] & 0xF];
//     }
//     out[j] = '\0';
// }

// void uartcobs_emit_py_tests() {
//     log_debug("STARTING emit py tests");
//     log_debug("%s", "__PY_CALL__:cobs_test");

//     const uint8_t p0[] = {};
//     const uint8_t p1[] = {0x00};
//     const uint8_t p2[] = {0x11, 0x00, 0x22, 0x14, 0x21, 0x15};
//     const uint8_t p3[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05};
//     const uint8_t p4[] = {0x00,0x00,0x00,0x01,0x00,0x02,0x00, 0x01,0x00,0x02,0x00, 0x01,0x00,0x02,0x00, 0x01,0x00,0x02,0x00};

//     const struct {
//         const uint8_t* data;
//         size_t len;
//     } cases[] = {
//         {p0, 0},
//         {p1, sizeof(p1)},
//         {p2, sizeof(p2)},
//         {p3, sizeof(p3)},
//         {p4, sizeof(p4)},
//     };

//     uint8_t encoded[512];
//     uint8_t decoded[512];           
//     size_t  decoded_len = 0;        

//     char payload_hex[512*2 + 1];
//     char encoded_hex[512*2 + 1];
//     char decoded_hex[512*2 + 1];    

//     for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
//         size_t enc_len = cobs_encode(cases[i].data, cases[i].len, encoded);

//         // Append delimiter for your UART framing, but DO NOT include it in decode input
//         encoded[enc_len++] = 0x00;

//         bytes_to_hex(cases[i].data, cases[i].len, payload_hex, sizeof(payload_hex));
//         bytes_to_hex(encoded, enc_len, encoded_hex, sizeof(encoded_hex));

//         log_debug("__COBS_ENC__:%s:%s", payload_hex, encoded_hex);

//         decoded_len = 0;
//         const bool ok = cobs_decode(
//             encoded,              // IMPORTANT: pass encoded bytes WITHOUT delimiter
//             enc_len - 1,          // exclude the trailing 0x00 delimiter
//             decoded,
//             sizeof(decoded),
//             &decoded_len
//         );

//         const bool match =
//             ok &&
//             (decoded_len == cases[i].len) &&
//             (cases[i].len == 0 || std::memcmp(decoded, cases[i].data, cases[i].len) == 0);

//         if (match) {
//             log_debug("__COBS_DEC__:PASS");
//         } else {
//             bytes_to_hex(decoded, decoded_len, decoded_hex, sizeof(decoded_hex));
//             log_debug("__COBS_DEC__:FAIL exp_len=%u got_len=%u got=%s",
//                       (unsigned)cases[i].len, (unsigned)decoded_len, decoded_hex);
//         }
//     }
// }