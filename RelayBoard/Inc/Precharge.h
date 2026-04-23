#ifndef RELAYBOARD_PRECHARGE_H
#define RELAYBOARD_PRECHARGE_H

#include <cstdint>

#include "AnalogIn.h"
#include "DigitalOut.h"

class Precharge
{
public:
    Precharge(
        DigitalOut& main_en,
        DigitalOut& precharge_en,
        AnalogIn& contactor12_voltage,
        AnalogIn& hal_effect_voltage);

    void run(uint16_t pack_voltage, bool cont12_fault, bool other_fault);
    uint8_t stage() const;
    uint16_t threshold() const;
    uint16_t hal_effect_millivolts() const;
    bool cont12_high() const;
    bool local_cont12_fault() const;

private:
    enum class State : uint8_t
    {
        WaitForHV = 0,
        WaitForEnable = 1,
        WaitForThreshold = 2,
        WaitForMainRelay = 3,
        Done = 4,
    };

    void fault_trap();
    void run_wait_for_hv_state();
    void run_wait_for_enable_state();
    void run_wait_for_threshold_state();
    void run_wait_for_main_relay_state();
    void run_done_state();
    uint16_t calculate_hal_effect_threshold_millivolts() const;

    DigitalOut& main_en_;
    DigitalOut& precharge_en_;
    AnalogIn& contactor12_voltage_;
    AnalogIn& hal_effect_voltage_;

    State state_ = State::WaitForEnable;
    uint16_t pack_voltage_ = 0;
    uint16_t threshold_millivolts_ = 0;
    uint16_t hal_effect_millivolts_ = 0;
    bool cont12_high_ = false;
    bool cont12_fault_ = false;
    bool local_cont12_fault_ = false;
    bool other_fault_ = false;
    bool fault_logged_ = false;
    bool timing_threshold_ = false;
    uint32_t threshold_start_time_ms_ = 0;
    uint32_t state_entry_time_ms_ = 0;
};

#endif
