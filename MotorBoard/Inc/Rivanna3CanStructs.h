
#ifndef rivanna3_CAN_Structs
#define rivanna3_CAN_Structs

#include "can.h"
#include "rivanna3.h"
#include "log.h"

typedef struct AuxBatteryStatus : CanMessage, rivanna3_aux_battery_status_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_aux_battery_status_pack(message->data, this,
            RIVANNA3_AUX_BATTERY_STATUS_LENGTH);
        message->len = RIVANNA3_AUX_BATTERY_STATUS_LENGTH;
        message->id = RIVANNA3_AUX_BATTERY_STATUS_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        rivanna3_aux_battery_status_unpack(this, message->data,
            RIVANNA3_AUX_BATTERY_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_AUX_BATTERY_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "AuxBatteryStatus: aux_voltage %u, percent_full %u",
            aux_voltage, percent_full);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} AuxBatteryStatus;


typedef struct MotorCommands : CanMessage, rivanna3_motor_commands_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_motor_commands_pack(message->data, this,
            RIVANNA3_MOTOR_COMMANDS_LENGTH);
        message->len = RIVANNA3_MOTOR_COMMANDS_LENGTH;
        message->id = RIVANNA3_MOTOR_COMMANDS_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        rivanna3_motor_commands_unpack(this, message->data,
            RIVANNA3_MOTOR_COMMANDS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_MOTOR_COMMANDS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MotorCommands: braking %u, regen_drive %u, manual_drive %u, cruise_drive %u, brake_pedal %u, throttle %u, cruise_speed %u, regen_braking %u, throttle_pedal %u",
            braking, regen_drive, manual_drive, cruise_drive, brake_pedal, throttle, cruise_speed, regen_braking, throttle_pedal);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MotorCommands;


typedef struct DashboardCommands : CanMessage, rivanna3_dashboard_commands_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_dashboard_commands_pack(message->data, this,
            RIVANNA3_DASHBOARD_COMMANDS_LENGTH);
        message->len = RIVANNA3_DASHBOARD_COMMANDS_LENGTH;
        message->id = RIVANNA3_DASHBOARD_COMMANDS_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        rivanna3_dashboard_commands_unpack(this, message->data,
            RIVANNA3_DASHBOARD_COMMANDS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_DASHBOARD_COMMANDS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "DashboardCommands: hazards %u, left_turn_signal %u, right_turn_signal %u, regen_en %u, cruise_inc %u, cruise_en %u, cruise_dec %u",
            hazards, left_turn_signal, right_turn_signal, regen_en, cruise_inc, cruise_en, cruise_dec);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} DashboardCommands;


// typedef struct Heartbeat : CanMessage, rivanna3_heartbeat_t {
//     void serialize(SerializedCanMessage *message) {
//         rivanna3_heartbeat_pack(message->data, this,
//             RIVANNA3_HEARTBEAT_LENGTH);
//         message->len = RIVANNA3_HEARTBEAT_LENGTH;
//         message->id = RIVANNA3_HEARTBEAT_FRAME_ID;
//     }

//     void deserialize(SerializedCanMessage *message) {
//         rivanna3_heartbeat_unpack(this, message->data,
//             RIVANNA3_HEARTBEAT_LENGTH);
//     }

//     static uint16_t get_message_ID() { return RIVANNA3_HEARTBEAT_FRAME_ID; }

//     void log_msg(LogLevel level) const {
//         log(level, __FILE__, __LINE__,
//             "Heartbeat: FromWheelBoard %u, FromPowerBoard %u, FromTelemetryBoard %u",
//             FromWheelBoard, FromPowerBoard, FromTelemetryBoard);
//     }

//     bool has_active_fault() {
//         return 0; // this message has no fault signals
//     }
// } Heartbeat;


// typedef struct ChargingMode : CanMessage, rivanna3_charging_mode_t {
//     void serialize(SerializedCanMessage *message) {
//         rivanna3_charging_mode_pack(message->data, this,
//             RIVANNA3_CHARGING_MODE_LENGTH);
//         message->len = RIVANNA3_CHARGING_MODE_LENGTH;
//         message->id = RIVANNA3_CHARGING_MODE_FRAME_ID;
//     }

//     void deserialize(SerializedCanMessage *message) {
//         rivanna3_charging_mode_unpack(this, message->data,
//             RIVANNA3_CHARGING_MODE_LENGTH);
//     }

//     static uint16_t get_message_ID() { return RIVANNA3_CHARGING_MODE_FRAME_ID; }

//     void log_msg(LogLevel level) const {
//         log(level, __FILE__, __LINE__,
//             "ChargingMode: ChargingModeEnable %u",
//             ChargingModeEnable);
//     }

//     bool has_active_fault() {
//         return 0; // this message has no fault signals
//     }
// } ChargingMode;

#endif