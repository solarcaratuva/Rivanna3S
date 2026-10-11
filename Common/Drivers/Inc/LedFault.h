#ifndef LED_FAULT_H
#define LED_FAULT_H

#include <stdint.h>
#include "stm32_hal.h"

// A pattern uses bits 0..3 for slow pulses and bits 4..7 for fast pulses
// on all configured fault LEDs together. A pause separates complete patterns.
enum LedFault_Faults {
    LedFault_HardFault     = 0x01, // one slow pulse
    LedFault_StackOverflow = 0x02, // two slow pulses
    LedFault_MallocFailed  = 0x03, // three slow pulses
    LedFault_MemManage     = 0x04, // four slow pulses
    LedFault_BusFault      = 0x05, // five slow pulses
    LedFault_UsageFault    = 0x06, // six slow pulses
    LedFault_NMI           = 0x07, // seven slow pulses
    LedFault_SecureFault   = 0x08, // eight slow pulses (U5)
    LedFault_RTOSAssert    = 0x11, // one slow pulse, then one fast pulse
};

#define LED_FAULT_SLOW_BLINK_MS 500U
#define LED_FAULT_FAST_BLINK_MS 200U
#define LED_FAULT_PATTERN_PAUSE_MS 1500U

#ifdef __cplusplus
extern "C" {
#endif

// Emit one pattern on already configured, active-high outputs in one port.
void LedFault_fault_signal_blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t pattern);

// Configure the board-defined LEDs directly, then repeat forever. The vendor
// HAL_Delay runs from a polled SysTick time base with interrupts disabled.
// All configured LEDs are ON before timing starts, even if the timer cannot run.
__attribute__((noreturn)) void LedFault_halt(enum LedFault_Faults fault);

#ifdef __cplusplus
}
#endif

#endif /* LED_FAULT_H */
