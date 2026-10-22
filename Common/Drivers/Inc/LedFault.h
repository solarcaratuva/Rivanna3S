#ifndef LED_FAULT_H
#define LED_FAULT_H

// The fault code is the number of blinks on PC1. A pause separates groups.
enum LedFault_Faults {
    LedFault_HardFault     = 1,
    LedFault_StackOverflow = 2,
    LedFault_MallocFailed  = 3,
    LedFault_MemManage     = 4,
    LedFault_BusFault      = 5,
    LedFault_UsageFault    = 6,
    LedFault_NMI           = 7,
    LedFault_SecureFault   = 8, // U5
    LedFault_RTOSAssert    = 9,
};

#ifdef __cplusplus
extern "C" {
#endif

// Turn on PC1 before the first delay, then repeat the fault code forever.
// Busy-wait timing works with interrupts disabled and does not use HAL/RTOS ticks.
__attribute__((noreturn)) void LedFault_halt(enum LedFault_Faults fault);

#ifdef __cplusplus
}
#endif

#endif /* LED_FAULT_H */
