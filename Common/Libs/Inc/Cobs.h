#ifndef COBS_H
#define COBS_H

#include <cstddef>
#include <cstdint>

/**
 * @brief Compute the maximum encoded output size for a given input length.
 *
 * For every 254 bytes of input, COBS inserts one extra overhead byte.
 * One leading overhead byte is always present, so:
 *   max_encoded_len = input_len + floor(input_len / 254) + 1
 *
 * The caller is responsible for appending a 0x00 packet delimiter after
 * the encoded data; that byte is NOT counted here.
 */
#define COBS_MAX_ENCODED_LEN(n) ((n) + ((n) / 254) + 1)

/**
 * @brief Encode a byte buffer using Consistent Overhead Byte Stuffing (COBS).
 *
 * @param[in]  input   Pointer to the raw input data.
 * @param[in]  length  Number of bytes in @p input.
 * @param[out] output  Destination buffer. Must be at least
 *                     COBS_MAX_ENCODED_LEN(length) bytes.
 *
 * @return Number of bytes written to @p output, not including any trailing
 *         0x00 delimiter.
 */
uint16_t cobs_encode(const uint8_t *input, uint16_t length, uint8_t *output);

/**
 * @brief Decode a COBS-encoded byte buffer (no delimiter), safely.
 *
 * Validates COBS codes, ensures it never reads past @p input, and never writes
 * more than @p output_capacity bytes.
 *
 * @param[in]  input            Pointer to the COBS-encoded data (no 0x00 delimiter).
 * @param[in]  length           Number of bytes in @p input.
 * @param[out] output           Destination buffer for decoded bytes.
 * @param[in]  output_capacity  Capacity of @p output in bytes.
 * @param[out] out_length       Number of decoded bytes written on success; set to 0 on failure.
 *
 * @return true on success, false on malformed input or insufficient output space.
 */
bool cobs_decode(const uint8_t *input,
                 uint16_t length,
                 uint8_t *output,
                 uint16_t output_capacity,
                 uint16_t *out_length);

#endif // COBS_H