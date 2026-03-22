
#ifndef rivanna3_s_CAN_Structs
#define rivanna3_s_CAN_Structs

#include "can.h"
#include "rivanna3_s.h"
#include "log.h"

typedef struct AuxBatteryStatus : CanMessage, rivanna3_s_aux_battery_status_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_aux_battery_status_pack(message->data, this,
            RIVANNA3_S_AUX_BATTERY_STATUS_LENGTH);
        message->len = RIVANNA3_S_AUX_BATTERY_STATUS_LENGTH;
        message->id = RIVANNA3_S_AUX_BATTERY_STATUS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_aux_battery_status_unpack(this, message->data,
            RIVANNA3_S_AUX_BATTERY_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_AUX_BATTERY_STATUS_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_AUX_BATTERY_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "AuxBatteryStatus: aux_voltage %u, percent_full %u",
            aux_voltage, percent_full);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} AuxBatteryStatus;


typedef struct MotorCommands : CanMessage, rivanna3_s_motor_commands_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_motor_commands_pack(message->data, this,
            RIVANNA3_S_MOTOR_COMMANDS_LENGTH);
        message->len = RIVANNA3_S_MOTOR_COMMANDS_LENGTH;
        message->id = RIVANNA3_S_MOTOR_COMMANDS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_motor_commands_unpack(this, message->data,
            RIVANNA3_S_MOTOR_COMMANDS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_MOTOR_COMMANDS_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_MOTOR_COMMANDS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MotorCommands: regen_braking %u, cruise_speed %u, throttle %u, manual_drive %u, regen_drive %u, cruise_drive %u",
            regen_braking, cruise_speed, throttle, manual_drive, regen_drive, cruise_drive);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MotorCommands;


typedef struct DashboardCommands : CanMessage, rivanna3_s_dashboard_commands_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_dashboard_commands_pack(message->data, this,
            RIVANNA3_S_DASHBOARD_COMMANDS_LENGTH);
        message->len = RIVANNA3_S_DASHBOARD_COMMANDS_LENGTH;
        message->id = RIVANNA3_S_DASHBOARD_COMMANDS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_dashboard_commands_unpack(this, message->data,
            RIVANNA3_S_DASHBOARD_COMMANDS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_DASHBOARD_COMMANDS_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_DASHBOARD_COMMANDS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "DashboardCommands: hazards %u, left_turn_signal %u, right_turn_signal %u, regen_en %u, cruise_inc %u, cruise_en %u, cruise_dec %u, charging_mode_en %u",
            hazards, left_turn_signal, right_turn_signal, regen_en, cruise_inc, cruise_en, cruise_dec, charging_mode_en);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} DashboardCommands;


typedef struct Heartbeat : CanMessage, rivanna3_s_heartbeat_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_heartbeat_pack(message->data, this,
            RIVANNA3_S_HEARTBEAT_LENGTH);
        message->len = RIVANNA3_S_HEARTBEAT_LENGTH;
        message->id = RIVANNA3_S_HEARTBEAT_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_heartbeat_unpack(this, message->data,
            RIVANNA3_S_HEARTBEAT_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_HEARTBEAT_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_HEARTBEAT_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "Heartbeat: source %u",
            source);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} Heartbeat;


typedef struct PedalStatus : CanMessage, rivanna3_s_pedal_status_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_pedal_status_pack(message->data, this,
            RIVANNA3_S_PEDAL_STATUS_LENGTH);
        message->len = RIVANNA3_S_PEDAL_STATUS_LENGTH;
        message->id = RIVANNA3_S_PEDAL_STATUS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_pedal_status_unpack(this, message->data,
            RIVANNA3_S_PEDAL_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_PEDAL_STATUS_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_PEDAL_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "PedalStatus: throttle_pedal %u, brake_pedal %u",
            throttle_pedal, brake_pedal);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} PedalStatus;


typedef struct PrechargeStatus : CanMessage, rivanna3_s_precharge_status_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_precharge_status_pack(message->data, this,
            RIVANNA3_S_PRECHARGE_STATUS_LENGTH);
        message->len = RIVANNA3_S_PRECHARGE_STATUS_LENGTH;
        message->id = RIVANNA3_S_PRECHARGE_STATUS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_precharge_status_unpack(this, message->data,
            RIVANNA3_S_PRECHARGE_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_PRECHARGE_STATUS_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_PRECHARGE_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "PrechargeStatus: motor_stage %u, mppt_stage %u, cont12_fault %u, other_fault %u, threshold %u, cont12 %u, hal_effect_motor %u, hal_effect_mppt %u",
            motor_stage, mppt_stage, cont12_fault, other_fault, threshold, cont12, hal_effect_motor, hal_effect_mppt);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} PrechargeStatus;


typedef struct Contactor12Error : CanMessage, rivanna3_s_contactor12_error_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_contactor12_error_pack(message->data, this,
            RIVANNA3_S_CONTACTOR12_ERROR_LENGTH);
        message->len = RIVANNA3_S_CONTACTOR12_ERROR_LENGTH;
        message->id = RIVANNA3_S_CONTACTOR12_ERROR_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_contactor12_error_unpack(this, message->data,
            RIVANNA3_S_CONTACTOR12_ERROR_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_CONTACTOR12_ERROR_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_CONTACTOR12_ERROR_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "Contactor12Error: cont12_went_low %u",
            cont12_went_low);
    }

    bool has_active_fault() {
        return cont12_went_low;
    }
} Contactor12Error;


typedef struct UpdateControl : CanMessage, rivanna3_s_update_control_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_update_control_pack(message->data, this,
            RIVANNA3_S_UPDATE_CONTROL_LENGTH);
        message->len = RIVANNA3_S_UPDATE_CONTROL_LENGTH;
        message->id = RIVANNA3_S_UPDATE_CONTROL_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_update_control_unpack(this, message->data,
            RIVANNA3_S_UPDATE_CONTROL_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_UPDATE_CONTROL_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_UPDATE_CONTROL_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "UpdateControl: target_board %u, setup %u, setup_ack %u, ready_for_data %u, done %u, final_crc %u",
            target_board, setup, setup_ack, ready_for_data, done, final_crc);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} UpdateControl;


typedef struct UpdateData : CanMessage, rivanna3_s_update_data_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_update_data_pack(message->data, this,
            RIVANNA3_S_UPDATE_DATA_LENGTH);
        message->len = RIVANNA3_S_UPDATE_DATA_LENGTH;
        message->id = RIVANNA3_S_UPDATE_DATA_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_update_data_unpack(this, message->data,
            RIVANNA3_S_UPDATE_DATA_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_UPDATE_DATA_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_UPDATE_DATA_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "UpdateData: data %u",
            data);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} UpdateData;

#endif