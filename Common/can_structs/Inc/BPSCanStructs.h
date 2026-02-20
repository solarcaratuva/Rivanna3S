
#ifndef bps_CAN_Structs
#define bps_CAN_Structs

#include "can.h"
#include "bps.h"
#include "log.h"

typedef struct BpsStatus : CanMessage, bps_bps_status_t {
    void serialize(SerializedCanMessage *message) {
        bps_bps_status_pack(message->data, this,
            BPS_BPS_STATUS_LENGTH);
        message->len = BPS_BPS_STATUS_LENGTH;
        message->id = BPS_BPS_STATUS_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        bps_bps_status_unpack(this, message->data,
            BPS_BPS_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return BPS_BPS_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "BpsStatus: pack_voltage %u, pack_current %u, pack_soc %u, discharge_relay_status %u, charge_relay_status %u, charger_safety %u, charge_power_signal %u, balancing_active %u",
            pack_voltage, pack_current, pack_soc, discharge_relay_status, charge_relay_status, charger_safety, charge_power_signal, balancing_active);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} BpsStatus;


typedef struct BpsError : CanMessage, bps_bps_error_t {
    void serialize(SerializedCanMessage *message) {
        bps_bps_error_pack(message->data, this,
            BPS_BPS_ERROR_LENGTH);
        message->len = BPS_BPS_ERROR_LENGTH;
        message->id = BPS_BPS_ERROR_FRAME_ID;
    }

    void deserialize(const SerializedCanMessage *message) {
        bps_bps_error_unpack(this, message->data,
            BPS_BPS_ERROR_LENGTH);
    }

    static uint16_t get_message_ID() { return BPS_BPS_ERROR_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "BpsError: internal_cell_communication_fault %u, weak_cell_fault %u, low_cell_voltage_fault %u, cell_open_wiring_fault %u, current_sensor_fault %u, weak_pack_fault %u, thermistor_fault %u, can_communication_fault %u, redundant_power_supply_fault %u, high_voltage_isolation_fault %u, charge_enable_relay_fault %u, discharge_enable_relay_fault %u, internal_hardware_fault %u, internal_heatsink_thermistor_fault %u, internal_logic_fault %u, highest_cell_voltage_too_high_fault %u, lowest_cell_voltage_too_low_fault %u, pack_too_hot_fault %u",
            internal_cell_communication_fault, weak_cell_fault, low_cell_voltage_fault, cell_open_wiring_fault, current_sensor_fault, weak_pack_fault, thermistor_fault, can_communication_fault, redundant_power_supply_fault, high_voltage_isolation_fault, charge_enable_relay_fault, discharge_enable_relay_fault, internal_hardware_fault, internal_heatsink_thermistor_fault, internal_logic_fault, highest_cell_voltage_too_high_fault, lowest_cell_voltage_too_low_fault, pack_too_hot_fault);
    }

    bool has_active_fault() {
        return internal_cell_communication_fault || low_cell_voltage_fault || current_sensor_fault || weak_pack_fault || thermistor_fault || can_communication_fault || redundant_power_supply_fault || high_voltage_isolation_fault || charge_enable_relay_fault || discharge_enable_relay_fault || internal_hardware_fault || internal_heatsink_thermistor_fault || internal_logic_fault || highest_cell_voltage_too_high_fault || lowest_cell_voltage_too_low_fault || pack_too_hot_fault;
    }
} BpsError;

#endif