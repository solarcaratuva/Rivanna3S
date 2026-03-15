#include "Cobs.h"
#include "log.h"

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>

static void log_bytes(const char* label, const uint8_t* data, size_t len) {
    log_debug("%s (len=%u):", label, (unsigned)len);

    if (len == 0) {
        log_debug("  <empty>");
        return;
    }

    char line[256];
    size_t pos = 0;

    for (size_t i = 0; i < len; ++i) {
        int written = std::snprintf(&line[pos],
                                    sizeof(line) - pos,
                                    "%02X ",
                                    data[i]);
        if (written < 0 || static_cast<size_t>(written) >= (sizeof(line) - pos)) {
            break;
        }
        pos += static_cast<size_t>(written);
    }

    log_debug("  %s", line);
}

size_t cobs_encode(const uint8_t *input, size_t length, uint8_t *output) {
    if (!input && length != 0) {
        log_debug("[cobs_encode] invalid input pointer");
        return 0;
    }
    if (!output) {
        log_debug("[cobs_encode] invalid output pointer");
        return 0;
    }

    const uint8_t *end = input + length;
    uint8_t *out_start = output;

    uint8_t *code_ptr = output++;   // reserve space for the first code byte
    uint8_t code = 1;

    log_debug("[cobs_encode] start length=%u", (unsigned)length);
    log_bytes("[cobs_encode] input", input, length);

    while (input < end) {
        if (*input == 0) {
            log_debug("[cobs_encode] saw zero, closing block with code=%u (0x%02X)",
                      (unsigned)code, code);

            *code_ptr = code;
            code_ptr = output++;    // reserve next code byte
            code = 1;
            ++input;
        } else {
            *output++ = *input++;
            ++code;

            if (code == 0xFF) {
                log_debug("[cobs_encode] max block reached, writing code=0xFF");
                *code_ptr = code;
                code_ptr = output++;
                code = 1;
            }
        }
    }

    *code_ptr = code;
    log_debug("[cobs_encode] final block code=%u (0x%02X) at output offset=%u",
              (unsigned)code,
              code,
              (unsigned)(code_ptr - out_start));

    size_t encoded_len = static_cast<size_t>(output - out_start);

    log_bytes("[cobs_encode] encoded", out_start, encoded_len);
    log_debug("[cobs_encode] end encoded_len=%u", (unsigned)encoded_len);

    return encoded_len;
}

bool cobs_decode(const uint8_t *input,
                 size_t length,
                 uint8_t *output,
                 size_t output_capacity,
                 size_t *out_length) {
    if (!out_length) {
        log_debug("[cobs_decode] out_length is null");
        return false;
    }
    *out_length = 0;

    if (!input && length != 0) {
        log_debug("[cobs_decode] invalid input pointer");
        return false;
    }

    if (!output && output_capacity != 0) {
        log_debug("[cobs_decode] invalid output pointer");
        return false;
    }

    log_debug("[cobs_decode] start length=%u output_capacity=%u",
              (unsigned)length, (unsigned)output_capacity);
    log_bytes("[cobs_decode] encoded input", input, length);

    const uint8_t *end = input + length;
    size_t out = 0;

    while (input < end) {
        uint8_t code = *input++;
        log_debug("[cobs_decode] read code=%u (0x%02X)", (unsigned)code, code);

        if (code == 0) {
            log_debug("[cobs_decode] invalid code byte 0");
            return false;
        }

        size_t copy_len = static_cast<size_t>(code) - 1;
        log_debug("[cobs_decode] copy_len=%u", (unsigned)copy_len);

        if (static_cast<size_t>(end - input) < copy_len) {
            log_debug("[cobs_decode] not enough input bytes remaining");
            return false;
        }

        if (out + copy_len > output_capacity) {
            log_debug("[cobs_decode] output buffer too small for copied bytes");
            return false;
        }

        for (size_t i = 0; i < copy_len; ++i) {
            output[out++] = *input++;
        }

        if (code < 0xFF && input < end) {
            if (out + 1 > output_capacity) {
                log_debug("[cobs_decode] output buffer too small for inserted zero");
                return false;
            }
            output[out++] = 0x00;
            log_debug("[cobs_decode] inserted implicit 00");
        }
    }

    *out_length = out;
    log_bytes("[cobs_decode] decoded", output, out);
    log_debug("[cobs_decode] end decoded_len=%u", (unsigned)out);

    return true;
}