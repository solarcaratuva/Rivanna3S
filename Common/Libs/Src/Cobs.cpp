#include "Cobs.h"
#include "log.h"

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>

uint16_t cobs_encode(const uint8_t *input, uint16_t length, uint8_t *output) {
    if (!input && length != 0) {
        log_warn("[cobs_encode] invalid input pointer");
        return 0;
    }
    if (!output) {
        log_warn("[cobs_encode] invalid output pointer");
        return 0;
    }

    const uint8_t *end = input + length;
    uint8_t *out_start = output;

    uint8_t *code_ptr = output++;   // reserve space for the first code byte
    uint8_t code = 1;

    while (input < end) {
        if (*input == 0) {

            *code_ptr = code;
            code_ptr = output++;    // reserve next code byte
            code = 1;
            ++input;
        } else {
            *output++ = *input++;
            ++code;

            if (code == 0xFF) {
                *code_ptr = code;
                code_ptr = output++;
                code = 1;
            }
        }
    }

    *code_ptr = code;

    uint16_t encoded_len = static_cast<uint16_t>(output - out_start);

    return encoded_len;
}

bool cobs_decode(const uint8_t *input,
                 uint16_t length,
                 uint8_t *output,
                 uint16_t output_capacity,
                 uint16_t *out_length) {

    const uint8_t *end = input + length;
    uint16_t out = 0;

    while (input < end) {
        uint8_t code = *input++;

        if (code == 0) {
            log_warn("[cobs_decode] invalid code byte 0");
            return false;
        }

        uint16_t copy_len = static_cast<uint16_t>(code) - 1;

        if (static_cast<uint16_t>(end - input) < copy_len) {
            log_warn("[cobs_decode] not enough input bytes remaining");
            return false;
        }

        if (out + copy_len > output_capacity) {
            log_warn("[cobs_decode] output buffer too small for copied bytes");
            return false;
        }

        for (uint16_t i = 0; i < copy_len; ++i) {
            output[out++] = *input++;
        }

        if (code < 0xFF && input < end) {
            if (out + 1 > output_capacity) {
                log_warn("[cobs_decode] output buffer too small for inserted zero");
                return false;
            }
            output[out++] = 0x00;
        }
    }

    *out_length = out;

    return true;
}