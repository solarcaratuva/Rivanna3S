
#ifndef motor_controller_CAN_Structs
#define motor_controller_CAN_Structs

#include "can.h"
#include "motor_controller.h"
#include "log.h"

typedef struct MotorControllerFrameRequest : CanMessage, motor_controller_motor_controller_frame_request_t {
    void serialize(SerializedCanMessage *message) {
        motor_controller_motor_controller_frame_request_pack(message->data, this,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_FRAME_REQUEST_LENGTH);
        message->len = MOTOR_CONTROLLER_MOTOR_CONTROLLER_FRAME_REQUEST_LENGTH;
        message->id = MOTOR_CONTROLLER_MOTOR_CONTROLLER_FRAME_REQUEST_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        motor_controller_motor_controller_frame_request_unpack(this, message->data,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_FRAME_REQUEST_LENGTH);
    }

    static uint16_t get_message_ID() { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_FRAME_REQUEST_FRAME_ID; }

    uint16_t ID() const { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_FRAME_REQUEST_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MotorControllerFrameRequest: power_status_frame %u, drive_status_frame %u, error_frame %u",
            power_status_frame, drive_status_frame, error_frame);
    }

    double get_power_status_frame() const { return motor_controller_motor_controller_frame_request_power_status_frame_decode(power_status_frame); }

    void set_power_status_frame(double value) { power_status_frame = motor_controller_motor_controller_frame_request_power_status_frame_encode(value); }

    bool power_status_frame_in_range() const { return motor_controller_motor_controller_frame_request_power_status_frame_is_in_range(power_status_frame); }

    double get_drive_status_frame() const { return motor_controller_motor_controller_frame_request_drive_status_frame_decode(drive_status_frame); }

    void set_drive_status_frame(double value) { drive_status_frame = motor_controller_motor_controller_frame_request_drive_status_frame_encode(value); }

    bool drive_status_frame_in_range() const { return motor_controller_motor_controller_frame_request_drive_status_frame_is_in_range(drive_status_frame); }

    double get_error_frame() const { return motor_controller_motor_controller_frame_request_error_frame_decode(error_frame); }

    void set_error_frame(double value) { error_frame = motor_controller_motor_controller_frame_request_error_frame_encode(value); }

    bool error_frame_in_range() const { return motor_controller_motor_controller_frame_request_error_frame_is_in_range(error_frame); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MotorControllerFrameRequest;


typedef struct MotorControllerPowerStatus : CanMessage, motor_controller_motor_controller_power_status_t {
    void serialize(SerializedCanMessage *message) {
        motor_controller_motor_controller_power_status_pack(message->data, this,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_POWER_STATUS_LENGTH);
        message->len = MOTOR_CONTROLLER_MOTOR_CONTROLLER_POWER_STATUS_LENGTH;
        message->id = MOTOR_CONTROLLER_MOTOR_CONTROLLER_POWER_STATUS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        motor_controller_motor_controller_power_status_unpack(this, message->data,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_POWER_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_POWER_STATUS_FRAME_ID; }

    uint16_t ID() const { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_POWER_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MotorControllerPowerStatus: battery_voltage %u, battery_current %u, battery_current_direction %u, motor_current %u, fet_temp %u, motor_rpm %u, pwm_duty %u, lead_angle %u",
            battery_voltage, battery_current, battery_current_direction, motor_current, fet_temp, motor_rpm, pwm_duty, lead_angle);
    }

    double get_battery_voltage() const { return motor_controller_motor_controller_power_status_battery_voltage_decode(battery_voltage); }

    void set_battery_voltage(double value) { battery_voltage = motor_controller_motor_controller_power_status_battery_voltage_encode(value); }

    bool battery_voltage_in_range() const { return motor_controller_motor_controller_power_status_battery_voltage_is_in_range(battery_voltage); }

    double get_battery_current() const { return motor_controller_motor_controller_power_status_battery_current_decode(battery_current); }

    void set_battery_current(double value) { battery_current = motor_controller_motor_controller_power_status_battery_current_encode(value); }

    bool battery_current_in_range() const { return motor_controller_motor_controller_power_status_battery_current_is_in_range(battery_current); }

    double get_battery_current_direction() const { return motor_controller_motor_controller_power_status_battery_current_direction_decode(battery_current_direction); }

    void set_battery_current_direction(double value) { battery_current_direction = motor_controller_motor_controller_power_status_battery_current_direction_encode(value); }

    bool battery_current_direction_in_range() const { return motor_controller_motor_controller_power_status_battery_current_direction_is_in_range(battery_current_direction); }

    double get_motor_current() const { return motor_controller_motor_controller_power_status_motor_current_decode(motor_current); }

    void set_motor_current(double value) { motor_current = motor_controller_motor_controller_power_status_motor_current_encode(value); }

    bool motor_current_in_range() const { return motor_controller_motor_controller_power_status_motor_current_is_in_range(motor_current); }

    double get_fet_temp() const { return motor_controller_motor_controller_power_status_fet_temp_decode(fet_temp); }

    void set_fet_temp(double value) { fet_temp = motor_controller_motor_controller_power_status_fet_temp_encode(value); }

    bool fet_temp_in_range() const { return motor_controller_motor_controller_power_status_fet_temp_is_in_range(fet_temp); }

    double get_motor_rpm() const { return motor_controller_motor_controller_power_status_motor_rpm_decode(motor_rpm); }

    void set_motor_rpm(double value) { motor_rpm = motor_controller_motor_controller_power_status_motor_rpm_encode(value); }

    bool motor_rpm_in_range() const { return motor_controller_motor_controller_power_status_motor_rpm_is_in_range(motor_rpm); }

    double get_pwm_duty() const { return motor_controller_motor_controller_power_status_pwm_duty_decode(pwm_duty); }

    void set_pwm_duty(double value) { pwm_duty = motor_controller_motor_controller_power_status_pwm_duty_encode(value); }

    bool pwm_duty_in_range() const { return motor_controller_motor_controller_power_status_pwm_duty_is_in_range(pwm_duty); }

    double get_lead_angle() const { return motor_controller_motor_controller_power_status_lead_angle_decode(lead_angle); }

    void set_lead_angle(double value) { lead_angle = motor_controller_motor_controller_power_status_lead_angle_encode(value); }

    bool lead_angle_in_range() const { return motor_controller_motor_controller_power_status_lead_angle_is_in_range(lead_angle); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MotorControllerPowerStatus;


typedef struct MotorControllerDriveStatus : CanMessage, motor_controller_motor_controller_drive_status_t {
    void serialize(SerializedCanMessage *message) {
        motor_controller_motor_controller_drive_status_pack(message->data, this,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_DRIVE_STATUS_LENGTH);
        message->len = MOTOR_CONTROLLER_MOTOR_CONTROLLER_DRIVE_STATUS_LENGTH;
        message->id = MOTOR_CONTROLLER_MOTOR_CONTROLLER_DRIVE_STATUS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        motor_controller_motor_controller_drive_status_unpack(this, message->data,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_DRIVE_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_DRIVE_STATUS_FRAME_ID; }

    uint16_t ID() const { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_DRIVE_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MotorControllerDriveStatus: power_mode %u, control_mode %u, accelerator_vr_position %u, regen_vr_position %u, digital_sw_position %u, output_target_value %u, motor_status %u, regen_status %u",
            power_mode, control_mode, accelerator_vr_position, regen_vr_position, digital_sw_position, output_target_value, motor_status, regen_status);
    }

    double get_power_mode() const { return motor_controller_motor_controller_drive_status_power_mode_decode(power_mode); }

    void set_power_mode(double value) { power_mode = motor_controller_motor_controller_drive_status_power_mode_encode(value); }

    bool power_mode_in_range() const { return motor_controller_motor_controller_drive_status_power_mode_is_in_range(power_mode); }

    double get_control_mode() const { return motor_controller_motor_controller_drive_status_control_mode_decode(control_mode); }

    void set_control_mode(double value) { control_mode = motor_controller_motor_controller_drive_status_control_mode_encode(value); }

    bool control_mode_in_range() const { return motor_controller_motor_controller_drive_status_control_mode_is_in_range(control_mode); }

    double get_accelerator_vr_position() const { return motor_controller_motor_controller_drive_status_accelerator_vr_position_decode(accelerator_vr_position); }

    void set_accelerator_vr_position(double value) { accelerator_vr_position = motor_controller_motor_controller_drive_status_accelerator_vr_position_encode(value); }

    bool accelerator_vr_position_in_range() const { return motor_controller_motor_controller_drive_status_accelerator_vr_position_is_in_range(accelerator_vr_position); }

    double get_regen_vr_position() const { return motor_controller_motor_controller_drive_status_regen_vr_position_decode(regen_vr_position); }

    void set_regen_vr_position(double value) { regen_vr_position = motor_controller_motor_controller_drive_status_regen_vr_position_encode(value); }

    bool regen_vr_position_in_range() const { return motor_controller_motor_controller_drive_status_regen_vr_position_is_in_range(regen_vr_position); }

    double get_digital_sw_position() const { return motor_controller_motor_controller_drive_status_digital_sw_position_decode(digital_sw_position); }

    void set_digital_sw_position(double value) { digital_sw_position = motor_controller_motor_controller_drive_status_digital_sw_position_encode(value); }

    bool digital_sw_position_in_range() const { return motor_controller_motor_controller_drive_status_digital_sw_position_is_in_range(digital_sw_position); }

    double get_output_target_value() const { return motor_controller_motor_controller_drive_status_output_target_value_decode(output_target_value); }

    void set_output_target_value(double value) { output_target_value = motor_controller_motor_controller_drive_status_output_target_value_encode(value); }

    bool output_target_value_in_range() const { return motor_controller_motor_controller_drive_status_output_target_value_is_in_range(output_target_value); }

    double get_motor_status() const { return motor_controller_motor_controller_drive_status_motor_status_decode(motor_status); }

    void set_motor_status(double value) { motor_status = motor_controller_motor_controller_drive_status_motor_status_encode(value); }

    bool motor_status_in_range() const { return motor_controller_motor_controller_drive_status_motor_status_is_in_range(motor_status); }

    double get_regen_status() const { return motor_controller_motor_controller_drive_status_regen_status_decode(regen_status); }

    void set_regen_status(double value) { regen_status = motor_controller_motor_controller_drive_status_regen_status_encode(value); }

    bool regen_status_in_range() const { return motor_controller_motor_controller_drive_status_regen_status_is_in_range(regen_status); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MotorControllerDriveStatus;


typedef struct MotorControllerError : CanMessage, motor_controller_motor_controller_error_t {
    void serialize(SerializedCanMessage *message) {
        motor_controller_motor_controller_error_pack(message->data, this,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_LENGTH);
        message->len = MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_LENGTH;
        message->id = MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        motor_controller_motor_controller_error_unpack(this, message->data,
            MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_LENGTH);
    }

    static uint16_t get_message_ID() { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_FRAME_ID; }

    uint16_t ID() const { return MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MotorControllerError: analog_sensor_err %u, motor_current_sensor_u_err %u, motor_current_sensor_w_err %u, fet_thermistor_err %u, battery_voltage_sensor_err %u, battery_current_sensor_err %u, battery_current_sensor_adj_err %u, motor_current_sensor_adj_err %u, accelerator_position_err %u, controller_voltage_sensor_err %u, power_system_err %u, overcurrent_err %u, overvoltage_err %u, overcurrent_limit %u, motor_system_err %u, motor_lock %u, hall_sensor_short %u, hall_sensor_open %u, overheat_level %u",
            analog_sensor_err, motor_current_sensor_u_err, motor_current_sensor_w_err, fet_thermistor_err, battery_voltage_sensor_err, battery_current_sensor_err, battery_current_sensor_adj_err, motor_current_sensor_adj_err, accelerator_position_err, controller_voltage_sensor_err, power_system_err, overcurrent_err, overvoltage_err, overcurrent_limit, motor_system_err, motor_lock, hall_sensor_short, hall_sensor_open, overheat_level);
    }

    double get_analog_sensor_err() const { return motor_controller_motor_controller_error_analog_sensor_err_decode(analog_sensor_err); }

    void set_analog_sensor_err(double value) { analog_sensor_err = motor_controller_motor_controller_error_analog_sensor_err_encode(value); }

    bool analog_sensor_err_in_range() const { return motor_controller_motor_controller_error_analog_sensor_err_is_in_range(analog_sensor_err); }

    double get_motor_current_sensor_u_err() const { return motor_controller_motor_controller_error_motor_current_sensor_u_err_decode(motor_current_sensor_u_err); }

    void set_motor_current_sensor_u_err(double value) { motor_current_sensor_u_err = motor_controller_motor_controller_error_motor_current_sensor_u_err_encode(value); }

    bool motor_current_sensor_u_err_in_range() const { return motor_controller_motor_controller_error_motor_current_sensor_u_err_is_in_range(motor_current_sensor_u_err); }

    double get_motor_current_sensor_w_err() const { return motor_controller_motor_controller_error_motor_current_sensor_w_err_decode(motor_current_sensor_w_err); }

    void set_motor_current_sensor_w_err(double value) { motor_current_sensor_w_err = motor_controller_motor_controller_error_motor_current_sensor_w_err_encode(value); }

    bool motor_current_sensor_w_err_in_range() const { return motor_controller_motor_controller_error_motor_current_sensor_w_err_is_in_range(motor_current_sensor_w_err); }

    double get_fet_thermistor_err() const { return motor_controller_motor_controller_error_fet_thermistor_err_decode(fet_thermistor_err); }

    void set_fet_thermistor_err(double value) { fet_thermistor_err = motor_controller_motor_controller_error_fet_thermistor_err_encode(value); }

    bool fet_thermistor_err_in_range() const { return motor_controller_motor_controller_error_fet_thermistor_err_is_in_range(fet_thermistor_err); }

    double get_battery_voltage_sensor_err() const { return motor_controller_motor_controller_error_battery_voltage_sensor_err_decode(battery_voltage_sensor_err); }

    void set_battery_voltage_sensor_err(double value) { battery_voltage_sensor_err = motor_controller_motor_controller_error_battery_voltage_sensor_err_encode(value); }

    bool battery_voltage_sensor_err_in_range() const { return motor_controller_motor_controller_error_battery_voltage_sensor_err_is_in_range(battery_voltage_sensor_err); }

    double get_battery_current_sensor_err() const { return motor_controller_motor_controller_error_battery_current_sensor_err_decode(battery_current_sensor_err); }

    void set_battery_current_sensor_err(double value) { battery_current_sensor_err = motor_controller_motor_controller_error_battery_current_sensor_err_encode(value); }

    bool battery_current_sensor_err_in_range() const { return motor_controller_motor_controller_error_battery_current_sensor_err_is_in_range(battery_current_sensor_err); }

    double get_battery_current_sensor_adj_err() const { return motor_controller_motor_controller_error_battery_current_sensor_adj_err_decode(battery_current_sensor_adj_err); }

    void set_battery_current_sensor_adj_err(double value) { battery_current_sensor_adj_err = motor_controller_motor_controller_error_battery_current_sensor_adj_err_encode(value); }

    bool battery_current_sensor_adj_err_in_range() const { return motor_controller_motor_controller_error_battery_current_sensor_adj_err_is_in_range(battery_current_sensor_adj_err); }

    double get_motor_current_sensor_adj_err() const { return motor_controller_motor_controller_error_motor_current_sensor_adj_err_decode(motor_current_sensor_adj_err); }

    void set_motor_current_sensor_adj_err(double value) { motor_current_sensor_adj_err = motor_controller_motor_controller_error_motor_current_sensor_adj_err_encode(value); }

    bool motor_current_sensor_adj_err_in_range() const { return motor_controller_motor_controller_error_motor_current_sensor_adj_err_is_in_range(motor_current_sensor_adj_err); }

    double get_accelerator_position_err() const { return motor_controller_motor_controller_error_accelerator_position_err_decode(accelerator_position_err); }

    void set_accelerator_position_err(double value) { accelerator_position_err = motor_controller_motor_controller_error_accelerator_position_err_encode(value); }

    bool accelerator_position_err_in_range() const { return motor_controller_motor_controller_error_accelerator_position_err_is_in_range(accelerator_position_err); }

    double get_controller_voltage_sensor_err() const { return motor_controller_motor_controller_error_controller_voltage_sensor_err_decode(controller_voltage_sensor_err); }

    void set_controller_voltage_sensor_err(double value) { controller_voltage_sensor_err = motor_controller_motor_controller_error_controller_voltage_sensor_err_encode(value); }

    bool controller_voltage_sensor_err_in_range() const { return motor_controller_motor_controller_error_controller_voltage_sensor_err_is_in_range(controller_voltage_sensor_err); }

    double get_power_system_err() const { return motor_controller_motor_controller_error_power_system_err_decode(power_system_err); }

    void set_power_system_err(double value) { power_system_err = motor_controller_motor_controller_error_power_system_err_encode(value); }

    bool power_system_err_in_range() const { return motor_controller_motor_controller_error_power_system_err_is_in_range(power_system_err); }

    double get_overcurrent_err() const { return motor_controller_motor_controller_error_overcurrent_err_decode(overcurrent_err); }

    void set_overcurrent_err(double value) { overcurrent_err = motor_controller_motor_controller_error_overcurrent_err_encode(value); }

    bool overcurrent_err_in_range() const { return motor_controller_motor_controller_error_overcurrent_err_is_in_range(overcurrent_err); }

    double get_overvoltage_err() const { return motor_controller_motor_controller_error_overvoltage_err_decode(overvoltage_err); }

    void set_overvoltage_err(double value) { overvoltage_err = motor_controller_motor_controller_error_overvoltage_err_encode(value); }

    bool overvoltage_err_in_range() const { return motor_controller_motor_controller_error_overvoltage_err_is_in_range(overvoltage_err); }

    double get_overcurrent_limit() const { return motor_controller_motor_controller_error_overcurrent_limit_decode(overcurrent_limit); }

    void set_overcurrent_limit(double value) { overcurrent_limit = motor_controller_motor_controller_error_overcurrent_limit_encode(value); }

    bool overcurrent_limit_in_range() const { return motor_controller_motor_controller_error_overcurrent_limit_is_in_range(overcurrent_limit); }

    double get_motor_system_err() const { return motor_controller_motor_controller_error_motor_system_err_decode(motor_system_err); }

    void set_motor_system_err(double value) { motor_system_err = motor_controller_motor_controller_error_motor_system_err_encode(value); }

    bool motor_system_err_in_range() const { return motor_controller_motor_controller_error_motor_system_err_is_in_range(motor_system_err); }

    double get_motor_lock() const { return motor_controller_motor_controller_error_motor_lock_decode(motor_lock); }

    void set_motor_lock(double value) { motor_lock = motor_controller_motor_controller_error_motor_lock_encode(value); }

    bool motor_lock_in_range() const { return motor_controller_motor_controller_error_motor_lock_is_in_range(motor_lock); }

    double get_hall_sensor_short() const { return motor_controller_motor_controller_error_hall_sensor_short_decode(hall_sensor_short); }

    void set_hall_sensor_short(double value) { hall_sensor_short = motor_controller_motor_controller_error_hall_sensor_short_encode(value); }

    bool hall_sensor_short_in_range() const { return motor_controller_motor_controller_error_hall_sensor_short_is_in_range(hall_sensor_short); }

    double get_hall_sensor_open() const { return motor_controller_motor_controller_error_hall_sensor_open_decode(hall_sensor_open); }

    void set_hall_sensor_open(double value) { hall_sensor_open = motor_controller_motor_controller_error_hall_sensor_open_encode(value); }

    bool hall_sensor_open_in_range() const { return motor_controller_motor_controller_error_hall_sensor_open_is_in_range(hall_sensor_open); }

    double get_overheat_level() const { return motor_controller_motor_controller_error_overheat_level_decode(overheat_level); }

    void set_overheat_level(double value) { overheat_level = motor_controller_motor_controller_error_overheat_level_encode(value); }

    bool overheat_level_in_range() const { return motor_controller_motor_controller_error_overheat_level_is_in_range(overheat_level); }

    bool has_active_fault() {
        return analog_sensor_err || motor_current_sensor_u_err || motor_current_sensor_w_err || fet_thermistor_err || battery_voltage_sensor_err || battery_current_sensor_err || battery_current_sensor_adj_err || motor_current_sensor_adj_err || accelerator_position_err || controller_voltage_sensor_err || power_system_err || overcurrent_err || overvoltage_err || overcurrent_limit || motor_system_err || motor_lock || hall_sensor_short || hall_sensor_open || overheat_level;
    }
} MotorControllerError;

#endif