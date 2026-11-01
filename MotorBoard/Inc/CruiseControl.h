#ifndef CRUISE_CONTROL_H
#define CRUISE_CONTROL_H

#include <cstdint>

class CruiseControl {
public:
    CruiseControl();

    void update_enabled_state(bool enabled, double current_speed_mph);
    bool should_control() const;

    void latch_on_brake();

    void increase_target();
    void decrease_target();

    uint8_t target_speed_command() const;
    double speed_from_motor_rpm(uint16_t motor_rpm) const;
    uint16_t compute_throttle(double current_speed_mph, uint32_t now_ms);

    void reset();

private:
    // Keep these aligned with the older cruise-control tuning until they are retuned on-car.
    static constexpr double kMotorRpmToMphRatio = 0.0596;
    static constexpr double kKp = 25.0;
    static constexpr double kKi = 0.1;
    static constexpr double kKd = 0.0;
    static constexpr double kIntegralMin = -500.0;
    static constexpr double kIntegralMax = 500.0;
    static constexpr double kMinOutput = 0.0;
    static constexpr double kMaxOutput = 150.0;
    static constexpr uint8_t kMinSpeedMph = 0;
    static constexpr uint8_t kMaxSpeedMph = 40;
    static constexpr uint8_t kSpeedStepMph = 5;

    bool enabled_;
    bool brake_latched_;
    double target_speed_mph_;
    double integral_;
    double previous_error_;
    uint32_t previous_time_ms_;

    void reset_controller_state();
    static double clamp_double(double value, double min_value, double max_value);
};

#endif
