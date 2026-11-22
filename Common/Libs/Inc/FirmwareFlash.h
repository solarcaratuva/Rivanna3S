#pragma once

#ifdef CAN_FIRMWARE_UPDATE
#include <stdint.h>

// Writes the upper address window only. Bank selection accounts for remapping;
// Activation is explicit and currently supported on G474 only.
class FirmwareFlash
{
public:
    bool configure(uint32_t image_size);
    uint32_t capacity() const { return capacity_; }
    bool erase();
    bool write(const uint8_t *data, uint16_t length);
    uint16_t crc() const;
    bool valid_image() const;
    bool activate();
    static uint32_t active_bank();
    void lock();

private:
    uint32_t base_ = 0;
    uint32_t bank_ = 0;
    uint32_t capacity_ = 0;
    uint32_t size_ = 0;
    uint32_t written_ = 0;
};
#endif
