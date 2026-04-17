#include "Precharge.h"

#include "Clock.h"
#include "log.h"
#include "pindef.h"

constexpr float CONT12_HIGH_THRESHOLD_V = 1.0f;
constexpr float PACK_VOLT_PCT = 0.9f;
constexpr float HAL_EFFECT_SENSITIVITY = 0.8f;
constexpr float VCC = 5.0f;
constexpr float PRECHARGE_RESISTANCE = 74.0f;
constexpr uint32_t HAL_EFFECT_BELOW_THRESHOLD_TIME_MS = 50;
constexpr uint32_t MAIN_RELAY_SETTLE_TIME_MS = 250;
constexpr uint32_t SAFETY_ENABLE_DELAY_MS = 3000;

DigitalOut safety_hv_enable(SAFETY_HV_EN);
DigitalOut safety_mtr_enable(SAFETY_MTR_EN);

Precharge::Precharge(
    DigitalOut &main_en,
    DigitalOut &precharge_en,
    AnalogIn &contactor12_voltage,
    AnalogIn &hal_effect_voltage)
    : main_en_(main_en),
      precharge_en_(precharge_en),
      contactor12_voltage_(contactor12_voltage),
      hal_effect_voltage_(hal_effect_voltage),
      state_(State::WaitForHV)
{
    main_en_.write(false);
    precharge_en_.write(true);
    state_entry_time_ms_ = Clock::get_current_time();
    safety_hv_enable.write(false);
    safety_mtr_enable.write(false);
}

void Precharge::run(uint16_t pack_voltage, bool cont12_fault, bool other_fault)
{
    cont12_high_ = contactor12_voltage_.read_voltage() > CONT12_HIGH_THRESHOLD_V;
    pack_voltage_ = pack_voltage;
    cont12_fault_ = cont12_fault;
    other_fault_ = other_fault;
    threshold_millivolts_ = calculate_hal_effect_threshold_millivolts();
    hal_effect_millivolts_ = static_cast<uint16_t>(hal_effect_voltage_.read_voltage() * 1000.0f);

    fault_trap();
    if (cont12_fault_ || other_fault_)
    {
        return;
    }
    if (state_ == State::WaitForHV)
    {
        run_wait_for_hv_state();
    }
    else if (state_ == State::WaitForEnable)
    {
        run_wait_for_enable_state();
    }
    else if (state_ == State::WaitForThreshold)
    {
        run_wait_for_threshold_state();
    }
    else if (state_ == State::WaitForMainRelay)
    {
        run_wait_for_main_relay_state();
    }
    else
    {
        run_done_state();
    }
}

uint8_t Precharge::stage() const
{
    return static_cast<uint8_t>(state_);
}

uint16_t Precharge::threshold() const
{
    return threshold_millivolts_;
}

uint16_t Precharge::hal_effect_millivolts() const
{
    return hal_effect_millivolts_;
}

bool Precharge::cont12_high() const
{
    return cont12_high_;
}

void Precharge::fault_trap()
{
    if (!cont12_high_ && state_ != State::WaitForHV && state_ != State::WaitForEnable)
    {
        cont12_fault_ = true;
    }

    if (!cont12_fault_ && !other_fault_)
    {
        fault_logged_ = false;
        return;
    }

    main_en_.write(false);
    precharge_en_.write(false);
    timing_threshold_ = false;
    state_ = State::WaitForHV;
    state_entry_time_ms_ = Clock::get_current_time();
    safety_hv_enable.write(false);
    safety_mtr_enable.write(false);

    if (!fault_logged_)
    {
        log_fault("Precharge disabled due to fault");
        fault_logged_ = true;
    }
}

void Precharge::run_wait_for_hv_state()
{
    main_en_.write(false);
    precharge_en_.write(true);
    safety_hv_enable.write(false);
    safety_mtr_enable.write(false);

    const uint32_t time_in_state_ms = Clock::get_current_time() - state_entry_time_ms_;
    if (time_in_state_ms >= SAFETY_ENABLE_DELAY_MS)
    {
        safety_mtr_enable.write(true);
        safety_hv_enable.write(true);
        state_ = State::WaitForEnable;
        state_entry_time_ms_ = Clock::get_current_time();
    }
}

void Precharge::run_wait_for_enable_state()
{
    if (cont12_high_ && pack_voltage_ != 0)
    {
        timing_threshold_ = false;
        state_ = State::WaitForThreshold;
        state_entry_time_ms_ = Clock::get_current_time();
    }
}

void Precharge::run_wait_for_threshold_state()
{
    if (hal_effect_millivolts_ < threshold_millivolts_)
    {
        if (!timing_threshold_)
        {
            timing_threshold_ = true;
            threshold_start_time_ms_ = Clock::get_current_time();
        }
    }
    else
    {
        timing_threshold_ = false;
    }

    if (timing_threshold_)
    {
        const uint32_t time_below_threshold_ms = Clock::get_current_time() - threshold_start_time_ms_;
        if (time_below_threshold_ms >= HAL_EFFECT_BELOW_THRESHOLD_TIME_MS)
        {
            main_en_.write(true);
            timing_threshold_ = false;
            state_ = State::WaitForMainRelay;
            state_entry_time_ms_ = Clock::get_current_time();
        }
    }
}

void Precharge::run_wait_for_main_relay_state()
{
    main_en_.write(true);

    const uint32_t time_in_state_ms = Clock::get_current_time() - state_entry_time_ms_;
    if (time_in_state_ms >= MAIN_RELAY_SETTLE_TIME_MS)
    {
        precharge_en_.write(false);
        state_ = State::Done;
    }
}

void Precharge::run_done_state()
{
    main_en_.write(true);
    precharge_en_.write(false);
}

uint16_t Precharge::calculate_hal_effect_threshold_millivolts() const
{
    float threshold =
        (((pack_voltage_ - pack_voltage_ * PACK_VOLT_PCT) / PRECHARGE_RESISTANCE) * HAL_EFFECT_SENSITIVITY
        + (VCC * 0.1f)) * (3.28f / VCC);

    return static_cast<uint16_t>(threshold * 1000.0f);
}
