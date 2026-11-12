#ifndef LED_FAULT_H
#define LED_FAULT_H

/**
 * @brief Fault codes signaled on the board debug LED.
 *
 * Each value corresponds to the number of LED blinks per cycle
 * before a pause and repeat.
 */
enum LedFault_Faults {
    LedFault_HardFault     = 1,
    LedFault_StackOverflow = 2,
    LedFault_MallocFailed  = 3,
    LedFault_MemManage     = 4,
    LedFault_BusFault      = 5,
    LedFault_UsageFault    = 6,
    LedFault_NMI           = 7,
    LedFault_SecureFault   = 8, // STM32U5 only
    LedFault_RTOSAssert    = 9,
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Disable interrupts and blink the fault code on the debug LED forever.
 *
 * Pin selected by the DEBUG_LED_* defines in LedFault.cpp. Best-effort after a
 * stack overflow: a blown SP can re-fault in the prologue and lock up instead.
 *
 * @param fault Fault code determining the number of blinks per cycle.
 */
__attribute__((noreturn)) void LedFault_halt(enum LedFault_Faults fault);

/**
 * @brief LedFault_halt(LedFault_RTOSAssert), as a plain void(void) entry point
 *        so configASSERT() does not need this header's enum.
 */
__attribute__((noreturn)) void LedFault_halt_assert(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_FAULT_H */
