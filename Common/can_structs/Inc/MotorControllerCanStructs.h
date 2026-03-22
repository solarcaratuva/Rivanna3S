
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

    bool has_active_fault() {
        return analog_sensor_err || motor_current_sensor_u_err || motor_current_sensor_w_err || fet_thermistor_err || battery_voltage_sensor_err || battery_current_sensor_err || battery_current_sensor_adj_err || motor_current_sensor_adj_err || accelerator_position_err || controller_voltage_sensor_err || power_system_err || overcurrent_err || overvoltage_err || overcurrent_limit || motor_system_err || motor_lock || hall_sensor_short || hall_sensor_open || overheat_level;
    }
} MotorControllerError;

#endif