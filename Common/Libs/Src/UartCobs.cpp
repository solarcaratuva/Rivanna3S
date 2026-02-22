#include "UartCobs.h"

UartCobs::UartCobs(UART *uart)
    : uart_(uart) {}

void UartCobs::write(const uint8_t *buffer, uint16_t length) {
    if (!uart_ || (!buffer && length != 0)) {
        return;
    }

    // Enforce documented contract
    if (length > MAX_PACKET_SIZE) {
        return;
    }

    size_t encoded_len = cobs_encode(buffer, static_cast<size_t>(length), tx_buf_);

    uart_->write(tx_buf_, static_cast<uint16_t>(encoded_len));

    uint8_t delimiter = 0x00;
    uart_->write(&delimiter, 1);
}

size_t UartCobs::read(uint8_t *buffer, uint16_t length) {
    if (!uart_ || (!buffer && length != 0)) {
        return 0;
    }

    size_t rx_len = 0;
    uint8_t byte = 0;

    // Read until delimiter OR until rx buffer fills up.
    while (rx_len < sizeof(rx_buf_)) {
        uart_->read(&byte, 1);  // assumes blocking read

        if (byte == 0x00) {
            break;
        }

        rx_buf_[rx_len++] = byte;
    }

    // If we filled rx_buf_ without seeing delimiter, the frame is too long.
    // Do not decode truncated data; drain until delimiter to resync.
    if (byte != 0x00) {
        do {
            uart_->read(&byte, 1);
        } while (byte != 0x00);

        return 0;
    }

    size_t decoded_len = 0;
    const bool ok = cobs_decode(rx_buf_,
                                rx_len,
                                buffer,
                                static_cast<size_t>(length),
                                &decoded_len);

    return ok ? decoded_len : 0;
}