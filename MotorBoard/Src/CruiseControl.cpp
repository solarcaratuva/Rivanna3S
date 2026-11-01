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
    return (uint8_t)(target_speed_mph_);
}

double CruiseControl::speed_from_motor_rpm(uint16_t motor_rpm) const
{
    return (double)(motor_rpm) * kMotorRpmToMphRatio;
}

uint16_t CruiseControl::compute_throttle(double current_speed_mph, uint32_t now_ms)
{
    if (!should_control()) {
        integral_ = 0.0;
        return 0;
    }

    const bool first_update = (previous_time_ms_ == 0);
    uint32_t dt_ms = now_ms - previous_time_ms_;
    previous_time_ms_ = now_ms;
    if (dt_ms > 20 || first_update) {
        dt_ms = 10;
    }
    if (dt_ms == 0) {
        dt_ms = 1;
    }

    const double error = target_speed_mph_ - current_speed_mph;
    const double proportional = kKp * error;
    integral_ += error * (double)(dt_ms);
    const double integral = kKi * integral_;
    const double derivative = (100.0 * (error - previous_error_)) / static_cast<double>(dt_ms);
    const double output = proportional + integral + kKd * derivative;

    previous_error_ = error;
    return (uint16_t)(clamp_double(output, kMinOutput, kMaxOutput));
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
