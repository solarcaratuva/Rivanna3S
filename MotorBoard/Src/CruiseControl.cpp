#include "CruiseControl.h"

CruiseControl::CruiseControl()
    : enabled_(false),
      brake_latched_(false),
      target_speed_mph_(0.0),
      integral_(0.0),
      previous_error_(0.0),
      previous_time_ms_(0)
{
}

void CruiseControl::update_enabled_state(bool enabled, double current_speed_mph)
{
    if (enabled && !enabled_) {
        enabled_ = true;
        brake_latched_ = false;
        target_speed_mph_ = clamp_double(current_speed_mph, kMinSpeedMph, kMaxSpeedMph);
        reset_controller_state();
        return;
    }

    if (!enabled && enabled_) {
        reset();
    }
}

bool CruiseControl::should_control() const
{
    return enabled_ && !brake_latched_;
}

void CruiseControl::latch_on_brake()
{
    brake_latched_ = true;
    reset_controller_state();
}

void CruiseControl::increase_target()
{
    target_speed_mph_ = clamp_double(target_speed_mph_ + kSpeedStepMph, kMinSpeedMph, kMaxSpeedMph);
}

void CruiseControl::decrease_target()
{
    target_speed_mph_ = clamp_double(target_speed_mph_ - kSpeedStepMph, kMinSpeedMph, kMaxSpeedMph);
}

uint8_t CruiseControl::target_speed_command() const
{
    return static_cast<uint8_t>(target_speed_mph_);
}

double CruiseControl::speed_from_motor_rpm(uint16_t motor_rpm) const
{
    return static_cast<double>(motor_rpm) * kMotorRpmToMphRatio;
}

uint16_t CruiseControl::compute_throttle(double current_speed_mph, uint32_t now_ms)
{
    if (!should_control()) {
        reset_controller_state();
        return 0;
    }

    const double error = target_speed_mph_ - current_speed_mph;

    if (previous_time_ms_ == 0 || now_ms <= previous_time_ms_) {
        previous_time_ms_ = now_ms;
        previous_error_ = error;
        const double output = clamp_double(kKp * error, kMinOutput, kMaxOutput);
        return static_cast<uint16_t>(output);
    }

    uint32_t dt_ms = now_ms - previous_time_ms_;
    if (dt_ms > 250) {
        dt_ms = 100;
    }
    if (dt_ms == 0) {
        dt_ms = 1;
    }

    const double dt_s = static_cast<double>(dt_ms) / 1000.0;

    integral_ += error * dt_s;
    integral_ = clamp_double(integral_, kIntegralMin, kIntegralMax);

    const double derivative = (error - previous_error_) / dt_s;
    const double output =
        kKp * error +
        kKi * integral_ +
        kKd * derivative;

    previous_error_ = error;
    previous_time_ms_ = now_ms;

    return static_cast<uint16_t>(clamp_double(output, kMinOutput, kMaxOutput));
}

void CruiseControl::reset()
{
    enabled_ = false;
    brake_latched_ = false;
    target_speed_mph_ = 0.0;
    reset_controller_state();
}

void CruiseControl::reset_controller_state()
{
    integral_ = 0.0;
    previous_error_ = 0.0;
    previous_time_ms_ = 0;
}

double CruiseControl::clamp_double(double value, double min_value, double max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}
