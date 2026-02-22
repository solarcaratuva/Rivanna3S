#include "Cobs.h"

size_t cobs_encode(const uint8_t *input, size_t length, uint8_t *output) {
    const uint8_t *end = input + length;
    uint8_t *out_start = output;

    uint8_t *code_ptr = output++;   // reserve space for the first code byte
    uint8_t code = 1;

    while (input < end) {
        if (*input == 0) {
            // End current block
            *code_ptr = code;
            code_ptr = output++;    // reserve next code byte
            code = 1;
            ++input;
        } else {
            // Copy non-zero byte
            *output++ = *input++;
            ++code;

            // If block is max length (254 bytes), start a new one
            if (code == 0xFF) {
                *code_ptr = code;
                code_ptr = output++;
                code = 1;
            }
        }
    }

    *code_ptr = code;
    return static_cast<size_t>(output - out_start);
}

bool cobs_decode(const uint8_t *input,
                 size_t length,
                 uint8_t *output,
                 size_t output_capacity,
                 size_t *out_length) {
    if (!out_length) return false;
    *out_length = 0;

    if (!input || (!output && output_capacity != 0)) return false;

    const uint8_t *end = input + length;
    size_t out = 0;

    while (input < end) {
        uint8_t code = *input++;

        // code must be 1..255 in valid COBS
        if (code == 0) return false;

        size_t copy_len = static_cast<size_t>(code) - 1;

        // Ensure encoded stream contains copy_len bytes to copy
        if (static_cast<size_t>(end - input) < copy_len) return false;

        // Ensure output buffer has space for copied bytes
        if (out + copy_len > output_capacity) return false;

        // Copy bytes
        for (size_t i = 0; i < copy_len; ++i) {
            output[out++] = *input++;
        }

        // Insert a 0 byte if this block ended due to an original 0,
        // i.e., code < 0xFF and we are not at the end of the encoded stream.
        if (code < 0xFF && input < end) {
            if (out + 1 > output_capacity) return false;
            output[out++] = 0x00;
        }
    }

    *out_length = out;
    return true;
}