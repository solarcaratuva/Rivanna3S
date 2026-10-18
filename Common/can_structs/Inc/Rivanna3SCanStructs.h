
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

    double get_aux_voltage() const { return rivanna3_s_aux_battery_status_aux_voltage_decode(aux_voltage); }

    void set_aux_voltage(double value) { aux_voltage = rivanna3_s_aux_battery_status_aux_voltage_encode(value); }

    bool aux_voltage_in_range() const { return rivanna3_s_aux_battery_status_aux_voltage_is_in_range(aux_voltage); }

    double get_percent_full() const { return rivanna3_s_aux_battery_status_percent_full_decode(percent_full); }

    void set_percent_full(double value) { percent_full = rivanna3_s_aux_battery_status_percent_full_encode(value); }

    bool percent_full_in_range() const { return rivanna3_s_aux_battery_status_percent_full_is_in_range(percent_full); }

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

    double get_regen_braking() const { return rivanna3_s_motor_commands_regen_braking_decode(regen_braking); }

    void set_regen_braking(double value) { regen_braking = rivanna3_s_motor_commands_regen_braking_encode(value); }

    bool regen_braking_in_range() const { return rivanna3_s_motor_commands_regen_braking_is_in_range(regen_braking); }

    double get_cruise_speed() const { return rivanna3_s_motor_commands_cruise_speed_decode(cruise_speed); }

    void set_cruise_speed(double value) { cruise_speed = rivanna3_s_motor_commands_cruise_speed_encode(value); }

    bool cruise_speed_in_range() const { return rivanna3_s_motor_commands_cruise_speed_is_in_range(cruise_speed); }

    double get_throttle() const { return rivanna3_s_motor_commands_throttle_decode(throttle); }

    void set_throttle(double value) { throttle = rivanna3_s_motor_commands_throttle_encode(value); }

    bool throttle_in_range() const { return rivanna3_s_motor_commands_throttle_is_in_range(throttle); }

    double get_manual_drive() const { return rivanna3_s_motor_commands_manual_drive_decode(manual_drive); }

    void set_manual_drive(double value) { manual_drive = rivanna3_s_motor_commands_manual_drive_encode(value); }

    bool manual_drive_in_range() const { return rivanna3_s_motor_commands_manual_drive_is_in_range(manual_drive); }

    double get_regen_drive() const { return rivanna3_s_motor_commands_regen_drive_decode(regen_drive); }

    void set_regen_drive(double value) { regen_drive = rivanna3_s_motor_commands_regen_drive_encode(value); }

    bool regen_drive_in_range() const { return rivanna3_s_motor_commands_regen_drive_is_in_range(regen_drive); }

    double get_cruise_drive() const { return rivanna3_s_motor_commands_cruise_drive_decode(cruise_drive); }

    void set_cruise_drive(double value) { cruise_drive = rivanna3_s_motor_commands_cruise_drive_encode(value); }

    bool cruise_drive_in_range() const { return rivanna3_s_motor_commands_cruise_drive_is_in_range(cruise_drive); }

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

    double get_hazards() const { return rivanna3_s_dashboard_commands_hazards_decode(hazards); }

    void set_hazards(double value) { hazards = rivanna3_s_dashboard_commands_hazards_encode(value); }

    bool hazards_in_range() const { return rivanna3_s_dashboard_commands_hazards_is_in_range(hazards); }

    double get_left_turn_signal() const { return rivanna3_s_dashboard_commands_left_turn_signal_decode(left_turn_signal); }

    void set_left_turn_signal(double value) { left_turn_signal = rivanna3_s_dashboard_commands_left_turn_signal_encode(value); }

    bool left_turn_signal_in_range() const { return rivanna3_s_dashboard_commands_left_turn_signal_is_in_range(left_turn_signal); }

    double get_right_turn_signal() const { return rivanna3_s_dashboard_commands_right_turn_signal_decode(right_turn_signal); }

    void set_right_turn_signal(double value) { right_turn_signal = rivanna3_s_dashboard_commands_right_turn_signal_encode(value); }

    bool right_turn_signal_in_range() const { return rivanna3_s_dashboard_commands_right_turn_signal_is_in_range(right_turn_signal); }

    double get_regen_en() const { return rivanna3_s_dashboard_commands_regen_en_decode(regen_en); }

    void set_regen_en(double value) { regen_en = rivanna3_s_dashboard_commands_regen_en_encode(value); }

    bool regen_en_in_range() const { return rivanna3_s_dashboard_commands_regen_en_is_in_range(regen_en); }

    double get_cruise_inc() const { return rivanna3_s_dashboard_commands_cruise_inc_decode(cruise_inc); }

    void set_cruise_inc(double value) { cruise_inc = rivanna3_s_dashboard_commands_cruise_inc_encode(value); }

    bool cruise_inc_in_range() const { return rivanna3_s_dashboard_commands_cruise_inc_is_in_range(cruise_inc); }

    double get_cruise_en() const { return rivanna3_s_dashboard_commands_cruise_en_decode(cruise_en); }

    void set_cruise_en(double value) { cruise_en = rivanna3_s_dashboard_commands_cruise_en_encode(value); }

    bool cruise_en_in_range() const { return rivanna3_s_dashboard_commands_cruise_en_is_in_range(cruise_en); }

    double get_cruise_dec() const { return rivanna3_s_dashboard_commands_cruise_dec_decode(cruise_dec); }

    void set_cruise_dec(double value) { cruise_dec = rivanna3_s_dashboard_commands_cruise_dec_encode(value); }

    bool cruise_dec_in_range() const { return rivanna3_s_dashboard_commands_cruise_dec_is_in_range(cruise_dec); }

    double get_charging_mode_en() const { return rivanna3_s_dashboard_commands_charging_mode_en_decode(charging_mode_en); }

    void set_charging_mode_en(double value) { charging_mode_en = rivanna3_s_dashboard_commands_charging_mode_en_encode(value); }

    bool charging_mode_en_in_range() const { return rivanna3_s_dashboard_commands_charging_mode_en_is_in_range(charging_mode_en); }

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

    double get_source() const { return rivanna3_s_heartbeat_source_decode(source); }

    void set_source(double value) { source = rivanna3_s_heartbeat_source_encode(value); }

    bool source_in_range() const { return rivanna3_s_heartbeat_source_is_in_range(source); }

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

    double get_throttle_pedal() const { return rivanna3_s_pedal_status_throttle_pedal_decode(throttle_pedal); }

    void set_throttle_pedal(double value) { throttle_pedal = rivanna3_s_pedal_status_throttle_pedal_encode(value); }

    bool throttle_pedal_in_range() const { return rivanna3_s_pedal_status_throttle_pedal_is_in_range(throttle_pedal); }

    double get_brake_pedal() const { return rivanna3_s_pedal_status_brake_pedal_decode(brake_pedal); }

    void set_brake_pedal(double value) { brake_pedal = rivanna3_s_pedal_status_brake_pedal_encode(value); }

    bool brake_pedal_in_range() const { return rivanna3_s_pedal_status_brake_pedal_is_in_range(brake_pedal); }

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

    double get_motor_stage() const { return rivanna3_s_precharge_status_motor_stage_decode(motor_stage); }

    void set_motor_stage(double value) { motor_stage = rivanna3_s_precharge_status_motor_stage_encode(value); }

    bool motor_stage_in_range() const { return rivanna3_s_precharge_status_motor_stage_is_in_range(motor_stage); }

    double get_mppt_stage() const { return rivanna3_s_precharge_status_mppt_stage_decode(mppt_stage); }

    void set_mppt_stage(double value) { mppt_stage = rivanna3_s_precharge_status_mppt_stage_encode(value); }

    bool mppt_stage_in_range() const { return rivanna3_s_precharge_status_mppt_stage_is_in_range(mppt_stage); }

    double get_cont12_fault() const { return rivanna3_s_precharge_status_cont12_fault_decode(cont12_fault); }

    void set_cont12_fault(double value) { cont12_fault = rivanna3_s_precharge_status_cont12_fault_encode(value); }

    bool cont12_fault_in_range() const { return rivanna3_s_precharge_status_cont12_fault_is_in_range(cont12_fault); }

    double get_other_fault() const { return rivanna3_s_precharge_status_other_fault_decode(other_fault); }

    void set_other_fault(double value) { other_fault = rivanna3_s_precharge_status_other_fault_encode(value); }

    bool other_fault_in_range() const { return rivanna3_s_precharge_status_other_fault_is_in_range(other_fault); }

    double get_threshold() const { return rivanna3_s_precharge_status_threshold_decode(threshold); }

    void set_threshold(double value) { threshold = rivanna3_s_precharge_status_threshold_encode(value); }

    bool threshold_in_range() const { return rivanna3_s_precharge_status_threshold_is_in_range(threshold); }

    double get_cont12() const { return rivanna3_s_precharge_status_cont12_decode(cont12); }

    void set_cont12(double value) { cont12 = rivanna3_s_precharge_status_cont12_encode(value); }

    bool cont12_in_range() const { return rivanna3_s_precharge_status_cont12_is_in_range(cont12); }

    double get_hal_effect_motor() const { return rivanna3_s_precharge_status_hal_effect_motor_decode(hal_effect_motor); }

    void set_hal_effect_motor(double value) { hal_effect_motor = rivanna3_s_precharge_status_hal_effect_motor_encode(value); }

    bool hal_effect_motor_in_range() const { return rivanna3_s_precharge_status_hal_effect_motor_is_in_range(hal_effect_motor); }

    double get_hal_effect_mppt() const { return rivanna3_s_precharge_status_hal_effect_mppt_decode(hal_effect_mppt); }

    void set_hal_effect_mppt(double value) { hal_effect_mppt = rivanna3_s_precharge_status_hal_effect_mppt_encode(value); }

    bool hal_effect_mppt_in_range() const { return rivanna3_s_precharge_status_hal_effect_mppt_is_in_range(hal_effect_mppt); }

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

    double get_cont12_went_low() const { return rivanna3_s_contactor12_error_cont12_went_low_decode(cont12_went_low); }

    void set_cont12_went_low(double value) { cont12_went_low = rivanna3_s_contactor12_error_cont12_went_low_encode(value); }

    bool cont12_went_low_in_range() const { return rivanna3_s_contactor12_error_cont12_went_low_is_in_range(cont12_went_low); }

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

    double get_target_board() const { return rivanna3_s_update_control_target_board_decode(target_board); }

    void set_target_board(double value) { target_board = rivanna3_s_update_control_target_board_encode(value); }

    bool target_board_in_range() const { return rivanna3_s_update_control_target_board_is_in_range(target_board); }

    double get_setup() const { return rivanna3_s_update_control_setup_decode(setup); }

    void set_setup(double value) { setup = rivanna3_s_update_control_setup_encode(value); }

    bool setup_in_range() const { return rivanna3_s_update_control_setup_is_in_range(setup); }

    double get_setup_ack() const { return rivanna3_s_update_control_setup_ack_decode(setup_ack); }

    void set_setup_ack(double value) { setup_ack = rivanna3_s_update_control_setup_ack_encode(value); }

    bool setup_ack_in_range() const { return rivanna3_s_update_control_setup_ack_is_in_range(setup_ack); }

    double get_ready_for_data() const { return rivanna3_s_update_control_ready_for_data_decode(ready_for_data); }

    void set_ready_for_data(double value) { ready_for_data = rivanna3_s_update_control_ready_for_data_encode(value); }

    bool ready_for_data_in_range() const { return rivanna3_s_update_control_ready_for_data_is_in_range(ready_for_data); }

    double get_done() const { return rivanna3_s_update_control_done_decode(done); }

    void set_done(double value) { done = rivanna3_s_update_control_done_encode(value); }

    bool done_in_range() const { return rivanna3_s_update_control_done_is_in_range(done); }

    double get_final_crc() const { return rivanna3_s_update_control_final_crc_decode(final_crc); }

    void set_final_crc(double value) { final_crc = rivanna3_s_update_control_final_crc_encode(value); }

    bool final_crc_in_range() const { return rivanna3_s_update_control_final_crc_is_in_range(final_crc); }

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

    double get_data() const { return rivanna3_s_update_data_data_decode(data); }

    void set_data(double value) { data = rivanna3_s_update_data_data_encode(value); }

    bool data_in_range() const { return rivanna3_s_update_data_data_is_in_range(data); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} UpdateData;


typedef struct OdometryData : CanMessage, rivanna3_s_odometry_data_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_odometry_data_pack(message->data, this,
            RIVANNA3_S_ODOMETRY_DATA_LENGTH);
        message->len = RIVANNA3_S_ODOMETRY_DATA_LENGTH;
        message->id = RIVANNA3_S_ODOMETRY_DATA_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_odometry_data_unpack(this, message->data,
            RIVANNA3_S_ODOMETRY_DATA_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_ODOMETRY_DATA_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_ODOMETRY_DATA_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "OdometryData: distance %u",
            distance);
    }

    double get_distance() const { return rivanna3_s_odometry_data_distance_decode(distance); }

    void set_distance(double value) { distance = rivanna3_s_odometry_data_distance_encode(value); }

    bool distance_in_range() const { return rivanna3_s_odometry_data_distance_is_in_range(distance); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} OdometryData;


typedef struct VN200Attitude : CanMessage, rivanna3_s_vn200_attitude_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_vn200_attitude_pack(message->data, this,
            RIVANNA3_S_VN200_ATTITUDE_LENGTH);
        message->len = RIVANNA3_S_VN200_ATTITUDE_LENGTH;
        message->id = RIVANNA3_S_VN200_ATTITUDE_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_vn200_attitude_unpack(this, message->data,
            RIVANNA3_S_VN200_ATTITUDE_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_VN200_ATTITUDE_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_VN200_ATTITUDE_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "VN200Attitude: yaw %u, pitch %u, roll %u, sequence %u",
            yaw, pitch, roll, sequence);
    }

    double get_yaw() const { return rivanna3_s_vn200_attitude_yaw_decode(yaw); }

    void set_yaw(double value) { yaw = rivanna3_s_vn200_attitude_yaw_encode(value); }

    bool yaw_in_range() const { return rivanna3_s_vn200_attitude_yaw_is_in_range(yaw); }

    double get_pitch() const { return rivanna3_s_vn200_attitude_pitch_decode(pitch); }

    void set_pitch(double value) { pitch = rivanna3_s_vn200_attitude_pitch_encode(value); }

    bool pitch_in_range() const { return rivanna3_s_vn200_attitude_pitch_is_in_range(pitch); }

    double get_roll() const { return rivanna3_s_vn200_attitude_roll_decode(roll); }

    void set_roll(double value) { roll = rivanna3_s_vn200_attitude_roll_encode(value); }

    bool roll_in_range() const { return rivanna3_s_vn200_attitude_roll_is_in_range(roll); }

    double get_sequence() const { return rivanna3_s_vn200_attitude_sequence_decode(sequence); }

    void set_sequence(double value) { sequence = rivanna3_s_vn200_attitude_sequence_encode(value); }

    bool sequence_in_range() const { return rivanna3_s_vn200_attitude_sequence_is_in_range(sequence); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} VN200Attitude;


typedef struct VN200AngularRate : CanMessage, rivanna3_s_vn200_angular_rate_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_vn200_angular_rate_pack(message->data, this,
            RIVANNA3_S_VN200_ANGULAR_RATE_LENGTH);
        message->len = RIVANNA3_S_VN200_ANGULAR_RATE_LENGTH;
        message->id = RIVANNA3_S_VN200_ANGULAR_RATE_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_vn200_angular_rate_unpack(this, message->data,
            RIVANNA3_S_VN200_ANGULAR_RATE_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_VN200_ANGULAR_RATE_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_VN200_ANGULAR_RATE_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "VN200AngularRate: gyro_x %u, gyro_y %u, gyro_z %u, sequence %u",
            gyro_x, gyro_y, gyro_z, sequence);
    }

    double get_gyro_x() const { return rivanna3_s_vn200_angular_rate_gyro_x_decode(gyro_x); }

    void set_gyro_x(double value) { gyro_x = rivanna3_s_vn200_angular_rate_gyro_x_encode(value); }

    bool gyro_x_in_range() const { return rivanna3_s_vn200_angular_rate_gyro_x_is_in_range(gyro_x); }

    double get_gyro_y() const { return rivanna3_s_vn200_angular_rate_gyro_y_decode(gyro_y); }

    void set_gyro_y(double value) { gyro_y = rivanna3_s_vn200_angular_rate_gyro_y_encode(value); }

    bool gyro_y_in_range() const { return rivanna3_s_vn200_angular_rate_gyro_y_is_in_range(gyro_y); }

    double get_gyro_z() const { return rivanna3_s_vn200_angular_rate_gyro_z_decode(gyro_z); }

    void set_gyro_z(double value) { gyro_z = rivanna3_s_vn200_angular_rate_gyro_z_encode(value); }

    bool gyro_z_in_range() const { return rivanna3_s_vn200_angular_rate_gyro_z_is_in_range(gyro_z); }

    double get_sequence() const { return rivanna3_s_vn200_angular_rate_sequence_decode(sequence); }

    void set_sequence(double value) { sequence = rivanna3_s_vn200_angular_rate_sequence_encode(value); }

    bool sequence_in_range() const { return rivanna3_s_vn200_angular_rate_sequence_is_in_range(sequence); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} VN200AngularRate;


typedef struct VN200Acceleration : CanMessage, rivanna3_s_vn200_acceleration_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_vn200_acceleration_pack(message->data, this,
            RIVANNA3_S_VN200_ACCELERATION_LENGTH);
        message->len = RIVANNA3_S_VN200_ACCELERATION_LENGTH;
        message->id = RIVANNA3_S_VN200_ACCELERATION_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_vn200_acceleration_unpack(this, message->data,
            RIVANNA3_S_VN200_ACCELERATION_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_VN200_ACCELERATION_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_VN200_ACCELERATION_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "VN200Acceleration: accel_x %u, accel_y %u, accel_z %u, sequence %u",
            accel_x, accel_y, accel_z, sequence);
    }

    double get_accel_x() const { return rivanna3_s_vn200_acceleration_accel_x_decode(accel_x); }

    void set_accel_x(double value) { accel_x = rivanna3_s_vn200_acceleration_accel_x_encode(value); }

    bool accel_x_in_range() const { return rivanna3_s_vn200_acceleration_accel_x_is_in_range(accel_x); }

    double get_accel_y() const { return rivanna3_s_vn200_acceleration_accel_y_decode(accel_y); }

    void set_accel_y(double value) { accel_y = rivanna3_s_vn200_acceleration_accel_y_encode(value); }

    bool accel_y_in_range() const { return rivanna3_s_vn200_acceleration_accel_y_is_in_range(accel_y); }

    double get_accel_z() const { return rivanna3_s_vn200_acceleration_accel_z_decode(accel_z); }

    void set_accel_z(double value) { accel_z = rivanna3_s_vn200_acceleration_accel_z_encode(value); }

    bool accel_z_in_range() const { return rivanna3_s_vn200_acceleration_accel_z_is_in_range(accel_z); }

    double get_sequence() const { return rivanna3_s_vn200_acceleration_sequence_decode(sequence); }

    void set_sequence(double value) { sequence = rivanna3_s_vn200_acceleration_sequence_encode(value); }

    bool sequence_in_range() const { return rivanna3_s_vn200_acceleration_sequence_is_in_range(sequence); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} VN200Acceleration;


typedef struct VN200Position : CanMessage, rivanna3_s_vn200_position_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_vn200_position_pack(message->data, this,
            RIVANNA3_S_VN200_POSITION_LENGTH);
        message->len = RIVANNA3_S_VN200_POSITION_LENGTH;
        message->id = RIVANNA3_S_VN200_POSITION_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_vn200_position_unpack(this, message->data,
            RIVANNA3_S_VN200_POSITION_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_VN200_POSITION_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_VN200_POSITION_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "VN200Position: latitude %u, longitude %u",
            latitude, longitude);
    }

    double get_latitude() const { return rivanna3_s_vn200_position_latitude_decode(latitude); }

    void set_latitude(double value) { latitude = rivanna3_s_vn200_position_latitude_encode(value); }

    bool latitude_in_range() const { return rivanna3_s_vn200_position_latitude_is_in_range(latitude); }

    double get_longitude() const { return rivanna3_s_vn200_position_longitude_decode(longitude); }

    void set_longitude(double value) { longitude = rivanna3_s_vn200_position_longitude_encode(value); }

    bool longitude_in_range() const { return rivanna3_s_vn200_position_longitude_is_in_range(longitude); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} VN200Position;


typedef struct VN200Velocity : CanMessage, rivanna3_s_vn200_velocity_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_vn200_velocity_pack(message->data, this,
            RIVANNA3_S_VN200_VELOCITY_LENGTH);
        message->len = RIVANNA3_S_VN200_VELOCITY_LENGTH;
        message->id = RIVANNA3_S_VN200_VELOCITY_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_vn200_velocity_unpack(this, message->data,
            RIVANNA3_S_VN200_VELOCITY_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_VN200_VELOCITY_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_VN200_VELOCITY_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "VN200Velocity: vel_n %u, vel_e %u, vel_d %u, num_sats %u, gnss_fix %u",
            vel_n, vel_e, vel_d, num_sats, gnss_fix);
    }

    double get_vel_n() const { return rivanna3_s_vn200_velocity_vel_n_decode(vel_n); }

    void set_vel_n(double value) { vel_n = rivanna3_s_vn200_velocity_vel_n_encode(value); }

    bool vel_n_in_range() const { return rivanna3_s_vn200_velocity_vel_n_is_in_range(vel_n); }

    double get_vel_e() const { return rivanna3_s_vn200_velocity_vel_e_decode(vel_e); }

    void set_vel_e(double value) { vel_e = rivanna3_s_vn200_velocity_vel_e_encode(value); }

    bool vel_e_in_range() const { return rivanna3_s_vn200_velocity_vel_e_is_in_range(vel_e); }

    double get_vel_d() const { return rivanna3_s_vn200_velocity_vel_d_decode(vel_d); }

    void set_vel_d(double value) { vel_d = rivanna3_s_vn200_velocity_vel_d_encode(value); }

    bool vel_d_in_range() const { return rivanna3_s_vn200_velocity_vel_d_is_in_range(vel_d); }

    double get_num_sats() const { return rivanna3_s_vn200_velocity_num_sats_decode(num_sats); }

    void set_num_sats(double value) { num_sats = rivanna3_s_vn200_velocity_num_sats_encode(value); }

    bool num_sats_in_range() const { return rivanna3_s_vn200_velocity_num_sats_is_in_range(num_sats); }

    double get_gnss_fix() const { return rivanna3_s_vn200_velocity_gnss_fix_decode(gnss_fix); }

    void set_gnss_fix(double value) { gnss_fix = rivanna3_s_vn200_velocity_gnss_fix_encode(value); }

    bool gnss_fix_in_range() const { return rivanna3_s_vn200_velocity_gnss_fix_is_in_range(gnss_fix); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} VN200Velocity;


typedef struct VN200Status : CanMessage, rivanna3_s_vn200_status_t {
    void serialize(SerializedCanMessage *message) {
        rivanna3_s_vn200_status_pack(message->data, this,
            RIVANNA3_S_VN200_STATUS_LENGTH);
        message->len = RIVANNA3_S_VN200_STATUS_LENGTH;
        message->id = RIVANNA3_S_VN200_STATUS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        rivanna3_s_vn200_status_unpack(this, message->data,
            RIVANNA3_S_VN200_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return RIVANNA3_S_VN200_STATUS_FRAME_ID; }

    uint16_t ID() const { return RIVANNA3_S_VN200_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "VN200Status: altitude %u, ins_status %u, time_status %u, sequence %u",
            altitude, ins_status, time_status, sequence);
    }

    double get_altitude() const { return rivanna3_s_vn200_status_altitude_decode(altitude); }

    void set_altitude(double value) { altitude = rivanna3_s_vn200_status_altitude_encode(value); }

    bool altitude_in_range() const { return rivanna3_s_vn200_status_altitude_is_in_range(altitude); }

    double get_ins_status() const { return rivanna3_s_vn200_status_ins_status_decode(ins_status); }

    void set_ins_status(double value) { ins_status = rivanna3_s_vn200_status_ins_status_encode(value); }

    bool ins_status_in_range() const { return rivanna3_s_vn200_status_ins_status_is_in_range(ins_status); }

    double get_time_status() const { return rivanna3_s_vn200_status_time_status_decode(time_status); }

    void set_time_status(double value) { time_status = rivanna3_s_vn200_status_time_status_encode(value); }

    bool time_status_in_range() const { return rivanna3_s_vn200_status_time_status_is_in_range(time_status); }

    double get_sequence() const { return rivanna3_s_vn200_status_sequence_decode(sequence); }

    void set_sequence(double value) { sequence = rivanna3_s_vn200_status_sequence_encode(value); }

    bool sequence_in_range() const { return rivanna3_s_vn200_status_sequence_is_in_range(sequence); }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} VN200Status;

#endif