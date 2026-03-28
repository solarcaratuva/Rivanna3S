#ifndef RELAYBOARD_PRECHARGE_H
#define RELAYBOARD_PRECHARGE_H

#include <cstdint>

#include "AnalogIn.h"
#include "DigitalOut.h"
#include "Rivanna3SCanStructs.h"

class Precharge
{
public:
    Precharge(DigitalOut& main_en, DigitalOut& precharge_en, AnalogIn& contactor12_voltage, AnalogIn& hal_effect_voltage);

    void run(uint16_t pack_voltage, bool cont12_fault, bool other_fault);
    PrechargeStatus& status();

private:
    enum class State : uint8_t
    {
        WaitForEnable = 0,
        WaitForThreshold = 1,
        WaitForMainRelay = 2,
    };

    void fault_trap();
    void run_wait_for_enable_state();
    void run_wait_for_threshold_state();
    void run_wait_for_main_relay_state();
    uint16_t calculate_hal_effect_threshold_millivolts() const;

    DigitalOut& main_en_;
    DigitalOut& precharge_en_;
    AnalogIn& contactor12_voltage_;
    AnalogIn& hal_effect_voltage_;

    PrechargeStatus status_{};
    State state_ = State::WaitForEnable;
    uint16_t pack_voltage_ = 0;
    bool cont12_high_ = false;
    bool cont12_fault_ = false;
    bool other_fault_ = false;
    bool fault_logged_ = false;
    bool timing_threshold_ = false;
    uint32_t threshold_start_time_ms_ = 0;
    uint32_t state_entry_time_ms_ = 0;
};

#endif
