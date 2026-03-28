#include "Precharge.h"

#include "Clock.h"
#include "log.h"

constexpr float CONT12_HIGH_THRESHOLD_V = 1.0f;
constexpr float PACK_VOLT_PCT = 0.9f;
constexpr float HAL_EFFECT_SENSITIVITY = 0.8f;
constexpr float VCC = 5.0f;
constexpr float PRECHARGE_RESISTANCE = 74.0f;
constexpr uint32_t HAL_EFFECT_BELOW_THRESHOLD_TIME_MS = 50;
constexpr uint32_t MAIN_RELAY_SETTLE_TIME_MS = 250;


Precharge::Precharge(DigitalOut &main_en, DigitalOut &precharge_en, AnalogIn &contactor12_voltage, AnalogIn &hal_effect_voltage)
    : main_en_(main_en),
      precharge_en_(precharge_en),
      contactor12_voltage_(contactor12_voltage),
      hal_effect_voltage_(hal_effect_voltage)
{

    main_en_.write(false);
    precharge_en_.write(true);

    status_ = PrechargeStatus{};
    status_.threshold = 0;
    status_.motor_stage = (uint8_t)state_;
}

void Precharge::run(uint16_t pack_voltage, bool cont12_fault, bool other_fault)
{
    cont12_high_ = contactor12_voltage_.read_voltage() > CONT12_HIGH_THRESHOLD_V;
    pack_voltage_ = pack_voltage;
    cont12_fault_ = cont12_fault;
    other_fault_ = other_fault;

    // Always refresh the outgoing CAN message so main.cpp can send it every loop.
    status_.cont12 = cont12_high_ ? 1 : 0;
    status_.threshold = calculate_hal_effect_threshold_millivolts();
    status_.hal_effect_motor = (uint16_t)(hal_effect_voltage_.read_voltage() * 1000.0f);
    status_.hal_effect_mppt = 0;

    // Any fault should immediately shut the relays off.
    fault_trap();
    status_.cont12_fault = cont12_fault_ ? 1 : 0;
    status_.other_fault = other_fault_ ? 1 : 0;
    if (status_.cont12_fault || status_.other_fault)
    {
        status_.motor_stage = (uint8_t)state_;
        return;
    }

    if (state_ == State::WaitForEnable)
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
    status_.motor_stage = (uint8_t)state_;
}

PrechargeStatus &Precharge::status()
{
    return status_;
}

void Precharge::fault_trap()
{
    if (!cont12_high_ && state_ != State::WaitForEnable)
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
    state_ = State::WaitForEnable;

    if (!fault_logged_)
    {
        log_fault("Precharge disabled due to fault");
        fault_logged_ = true;
    }
}

void Precharge::run_wait_for_enable_state()
{
    // Step 0:
    // Wait until Contactor 12 is high and the BPS says pack voltage is present.
    if (cont12_high_ && pack_voltage_ != 0)
    {
        timing_threshold_ = false;
        state_ = State::WaitForThreshold;
        state_entry_time_ms_ = Clock::get_current_time();
    }
}

void Precharge::run_wait_for_threshold_state()
{
    // Step 1:
    // Start a timer when the hall-effect reading goes below the threshold.
    // If it rises above the threshold, reset the timer.
    if (status_.hal_effect_motor < status_.threshold)
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
    // Step 2:
    // Keep the main relay on. Once enough time has passed, turn the
    // precharge relay off and stay in that final relay configuration.
    main_en_.write(true);

    const uint32_t time_in_state_ms = Clock::get_current_time() - state_entry_time_ms_;
    if (time_in_state_ms >= MAIN_RELAY_SETTLE_TIME_MS)
    {
        precharge_en_.write(false);
    }
}

uint16_t Precharge::calculate_hal_effect_threshold_millivolts() const
{
    float threshold =
        (((pack_voltage_ - pack_voltage_ * PACK_VOLT_PCT) / PRECHARGE_RESISTANCE) * HAL_EFFECT_SENSITIVITY
        + (VCC * 0.1f)) * (3.28f / VCC);

    return (uint16_t)(threshold * 1000.0f);
}

