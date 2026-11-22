#include "FirmwareFlash.h"

#ifdef CAN_FIRMWARE_UPDATE
#include "FirmwareUploader.h"
#include "stm32_hal.h"
#include <cstring>

bool FirmwareFlash::configure(uint32_t image_size)
{
    capacity_ = 0;
#if defined(STM32G474xx)
    if (!(FLASH->OPTR & FLASH_OPTR_DBANK) ||
        *reinterpret_cast<volatile const uint16_t *>(FLASHSIZE_BASE) != 512) {
        return false;
    }
    capacity_ = 256 * 1024;
    bank_ = (SYSCFG->MEMRMP & SYSCFG_MEMRMP_FB_MODE) ? FLASH_BANK_1 : FLASH_BANK_2;
#elif defined(STM32H743xx)
    if (*reinterpret_cast<volatile const uint16_t *>(FLASHSIZE_BASE) != 2048) return false;
    capacity_ = 1024 * 1024;
    bank_ = (FLASH->OPTCR & FLASH_OPTCR_SWAP_BANK) ? FLASH_BANK_1 : FLASH_BANK_2;
#endif
    // The updater and its vector table must both reside in the lower window.
    // The update build's linker limit also keeps all application sections there.
    const uintptr_t code_address = reinterpret_cast<uintptr_t>(&FirmwareUploader::crc16_hqx);
    if (!capacity_ || code_address < FLASH_BASE || code_address >= FLASH_BASE + capacity_ ||
        SCB->VTOR < FLASH_BASE || SCB->VTOR >= FLASH_BASE + capacity_) {
        capacity_ = 0;
        return false;
    }
    base_ = FLASH_BASE + capacity_;
    size_ = image_size;
    written_ = 0;
    return size_ && size_ <= capacity_;
}

bool FirmwareFlash::erase()
{
    if (HAL_FLASH_Unlock() != HAL_OK) return false;
    FLASH_EraseInitTypeDef erase{};
    erase.TypeErase = FLASH_TYPEERASE_MASSERASE;
    erase.Banks = bank_;
#if defined(STM32H743xx)
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
#endif
    uint32_t error = 0;
    // The polling HAL waits for completion and handles the flash cache state.
    const bool ok = HAL_FLASHEx_Erase(&erase, &error) == HAL_OK;
    lock();
    return ok;
}

bool FirmwareFlash::write(const uint8_t *data, uint16_t length)
{
    if (!length || length > FW_BLOCK_SIZE || written_ + length > size_) return false;
    // Every block except the last is 64 bytes. Pad the last programming unit,
    // but exclude that padding from the image size and final CRC.
#if defined(STM32G474xx)
    constexpr uint32_t program_size = 8;
#else
    constexpr uint32_t program_size = 32;
#endif
    if (written_ % program_size || (length != FW_BLOCK_SIZE && written_ + length != size_)) return false;
    if (HAL_FLASH_Unlock() != HAL_OK) return false;
    bool ok = true;
    for (uint32_t offset = 0; offset < length && ok; offset += program_size) {
        alignas(32) uint8_t word[program_size];
        std::memset(word, 0xff, sizeof(word));
        const uint32_t remaining = length - offset;
        std::memcpy(word, data + offset, remaining < program_size ? remaining : program_size);
#if defined(STM32G474xx)
        uint64_t value;
        std::memcpy(&value, word, sizeof(value));
        ok = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, base_ + written_ + offset, value) == HAL_OK;
#else
        ok = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, base_ + written_ + offset,
                               reinterpret_cast<uint32_t>(word)) == HAL_OK;
#endif
    }
    lock();
#if defined(STM32H743xx)
    // H7 data-cache lines may still contain data from before the erase.
    SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t *>(base_ + written_),
                               ((length + 31) / 32) * 32);
#endif
    if (ok) {
        ok = std::memcmp(reinterpret_cast<const void *>(base_ + written_), data, length) == 0;
    }
    if (ok) written_ += length;
    return ok;
}

uint16_t FirmwareFlash::crc() const
{
    uint16_t result = FW_CRC16_INIT;
    for (uint32_t offset = 0; offset < written_; offset += FW_BLOCK_SIZE) {
        const uint32_t remaining = written_ - offset;
        result = FirmwareUploader::crc16_hqx(reinterpret_cast<const uint8_t *>(base_ + offset),
                    remaining < FW_BLOCK_SIZE ? remaining : FW_BLOCK_SIZE, result);
    }
    return result;
}

void FirmwareFlash::lock()
{
    HAL_FLASH_Lock();
}

uint32_t FirmwareFlash::active_bank()
{
#if defined(STM32G474xx)
    return (SYSCFG->MEMRMP & SYSCFG_MEMRMP_FB_MODE) ? 2 : 1;
#else
    return 0; // Activation has not been validated on H743.
#endif
}

bool FirmwareFlash::valid_image() const
{
#if defined(STM32G474xx)
    if (written_ != size_ || size_ < 8 || !capacity_) return false;
    const auto *vectors = reinterpret_cast<const uint32_t *>(base_);
    const uint32_t stack = vectors[0];
    const uint32_t reset = vectors[1];
    return !(stack & 7) && stack > SRAM_BASE && stack <= SRAM_BASE + 128 * 1024 &&
           (reset & 1) && (reset & ~1U) >= FLASH_BASE && (reset & ~1U) < FLASH_BASE + size_;
#else
    return false;
#endif
}

bool FirmwareFlash::activate()
{
#if defined(STM32G474xx)
    if (!valid_image() || HAL_FLASH_Unlock() != HAL_OK) return false;
    if (HAL_FLASH_OB_Unlock() != HAL_OK) {
        lock();
        return false;
    }
    FLASH_OBProgramInitTypeDef options{};
    options.OptionType = OPTIONBYTE_USER;
    options.USERType = OB_USER_BFB2;
    options.USERConfig = bank_ == FLASH_BANK_2 ? OB_BFB2_ENABLE : OB_BFB2_DISABLE;
    if (HAL_FLASHEx_OBProgram(&options) == HAL_OK) {
        HAL_FLASH_OB_Launch(); // Successful option reload resets the MCU.
    }
    HAL_FLASH_OB_Lock();
    lock();
#endif
    return false; // A successful activation does not return.
}
#endif
