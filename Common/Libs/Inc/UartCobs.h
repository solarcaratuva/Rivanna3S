#ifndef UARTCOBS_H
#define UARTCOBS_H

#include <cstddef>
#include <cstdint>

#include "UART.h"
#include "Cobs.h"



/**
 * @class UartCobs
 * @brief UART communication with COBS framing.
 *
 * Wraps a UART instance to provide framed, delimiter-safe messaging using
 * Consistent Overhead Byte Stuffing (COBS). Each transmitted message is
 * COBS-encoded (guaranteeing no 0x00 bytes in the payload) and terminated
 * with a 0x00 packet delimiter. On receive, bytes are accumulated until the
 * 0x00 delimiter is seen, then COBS-decoded into the caller's buffer.
 *
 * No dynamic memory allocation is used; internal encode/decode buffers are
 * fixed at compile time via MAX_PACKET_SIZE.
 *
 * Example usage:
 * @code
 * UART serial(PA_2, PA_3, 115200);
 * UartCobs cobs_serial(&serial);
 *
 * // Send a message
 * uint8_t payload[] = {0x01, 0x00, 0x02};
 * cobs_serial.write(payload, sizeof(payload));
 *
 * // Receive a message
 * uint8_t buf[UartCobs::MAX_PACKET_SIZE];
 * uint16_t len = cobs_serial.read(buf, sizeof(buf));
 * @endcode
 */
class UartCobs {
public:
    /**
     * @brief Maximum raw (unencoded) payload size in bytes.
     *
     * Both the transmit and receive internal buffers are sized to accommodate
     * a COBS-encoded frame of this length. Adjust as needed if we change the payload size
     */
    static constexpr uint16_t MAX_PACKET_SIZE = 256;

    /**
     * @brief Construct a UartCobs instance wrapping an existing UART object.
     *
     * @param uart Pointer to an already-initialized UART instance. Must not be
     *             nullptr and must remain valid for the lifetime of this object.
     */
    explicit UartCobs(UART *uart);

    /**
     * @brief COBS-encode a message and transmit it over UART.
     *
     * Encodes @p buffer using COBS (ensuring no 0x00 bytes appear in the
     * output), writes the encoded bytes over UART, then writes a single 0x00
     * packet delimiter to mark the end of the frame.
     *
     * @param[in] buffer Pointer to the raw data to send.
     * @param[in] length Number of bytes in @p buffer. Must be ≤ MAX_PACKET_SIZE.
     */
    void write(const uint8_t *buffer, uint16_t length);

    /**
     * @brief Receive a COBS-framed message from UART and decode it.
     *
     * Reads one byte at a time (blocking) until a 0x00 delimiter is received,
     * accumulating the COBS-encoded frame in an internal buffer. The frame is
     * then COBS-decoded and the result is copied into @p buffer.
     *
     * If the incoming frame is too large, malformed, or does not fit in @p buffer,
     * this returns 0 and discards bytes until the next delimiter to resynchronize.
     *
     * @param[out] buffer Destination for the decoded message.
     * @param[in]  length Capacity of @p buffer in bytes.
     *
     * @return Number of decoded bytes written to @p buffer, or 0 on error.
     */
    uint16_t read(uint8_t *buffer, uint16_t length);


private:
    UART *uart_;

    /// Internal buffer for COBS-encoding outgoing payloads.
    uint8_t tx_buf_[COBS_MAX_ENCODED_LEN(MAX_PACKET_SIZE)];

    /// Internal buffer for accumulating incoming COBS-encoded frames.
    uint8_t rx_buf_[COBS_MAX_ENCODED_LEN(MAX_PACKET_SIZE)];
};

#endif // UARTCOBS_H