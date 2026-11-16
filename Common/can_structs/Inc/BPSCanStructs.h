
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

    uint16_t ID() const { return BPS_BPS_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "BpsStatus: pack_voltage %u, pack_current %u, pack_soc %u, discharge_relay_status %u, charge_relay_status %u, charger_safety %u, charge_power_signal %u, balancing_active %u",
            pack_voltage, pack_current, pack_soc, discharge_relay_status, charge_relay_status, charger_safety, charge_power_signal, balancing_active);
    }

    double get_pack_voltage() const { return bps_bps_status_pack_voltage_decode(pack_voltage); }

    void set_pack_voltage(double value) { pack_voltage = bps_bps_status_pack_voltage_encode(value); }

    bool pack_voltage_in_range() const { return bps_bps_status_pack_voltage_is_in_range(pack_voltage); }

    double get_pack_current() const { return bps_bps_status_pack_current_decode(pack_current); }

    void set_pack_current(double value) { pack_current = bps_bps_status_pack_current_encode(value); }

    bool pack_current_in_range() const { return bps_bps_status_pack_current_is_in_range(pack_current); }

    double get_pack_soc() const { return bps_bps_status_pack_soc_decode(pack_soc); }

    void set_pack_soc(double value) { pack_soc = bps_bps_status_pack_soc_encode(value); }

    bool pack_soc_in_range() const { return bps_bps_status_pack_soc_is_in_range(pack_soc); }

    double get_discharge_relay_status() const { return bps_bps_status_discharge_relay_status_decode(discharge_relay_status); }

    void set_discharge_relay_status(double value) { discharge_relay_status = bps_bps_status_discharge_relay_status_encode(value); }

    bool discharge_relay_status_in_range() const { return bps_bps_status_discharge_relay_status_is_in_range(discharge_relay_status); }

    double get_charge_relay_status() const { return bps_bps_status_charge_relay_status_decode(charge_relay_status); }

    void set_charge_relay_status(double value) { charge_relay_status = bps_bps_status_charge_relay_status_encode(value); }

    bool charge_relay_status_in_range() const { return bps_bps_status_charge_relay_status_is_in_range(charge_relay_status); }

    double get_charger_safety() const { return bps_bps_status_charger_safety_decode(charger_safety); }

    void set_charger_safety(double value) { charger_safety = bps_bps_status_charger_safety_encode(value); }

    bool charger_safety_in_range() const { return bps_bps_status_charger_safety_is_in_range(charger_safety); }

    double get_charge_power_signal() const { return bps_bps_status_charge_power_signal_decode(charge_power_signal); }

    void set_charge_power_signal(double value) { charge_power_signal = bps_bps_status_charge_power_signal_encode(value); }

    bool charge_power_signal_in_range() const { return bps_bps_status_charge_power_signal_is_in_range(charge_power_signal); }

    double get_balancing_active() const { return bps_bps_status_balancing_active_decode(balancing_active); }

    void set_balancing_active(double value) { balancing_active = bps_bps_status_balancing_active_encode(value); }

    bool balancing_active_in_range() const { return bps_bps_status_balancing_active_is_in_range(balancing_active); }

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

    uint16_t ID() const { return BPS_BPS_ERROR_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "BpsError: internal_cell_communication_fault %u, weak_cell_fault %u, low_cell_voltage_fault %u, cell_open_wiring_fault %u, thermistor_fault %u, current_sensor_fault %u, weak_pack_fault %u, can_communication_fault %u, redundant_power_supply_fault %u, high_voltage_isolation_fault %u, charge_enable_relay_fault %u, discharge_enable_relay_fault %u, internal_conversion_fault %u, internal_memory_fault %u, internal_thermistor_fault %u, internal_logic_fault %u, reduced_low_soc %u, reduced_high_cell_resistance %u, reduced_high_temp %u, reduced_low_cell_voltage %u, reduced_low_pack_voltage %u, reduced_voltage_failsafe %u, reduced_comms_failsafe %u",
            internal_cell_communication_fault, weak_cell_fault, low_cell_voltage_fault, cell_open_wiring_fault, thermistor_fault, current_sensor_fault, weak_pack_fault, can_communication_fault, redundant_power_supply_fault, high_voltage_isolation_fault, charge_enable_relay_fault, discharge_enable_relay_fault, internal_conversion_fault, internal_memory_fault, internal_thermistor_fault, internal_logic_fault, reduced_low_soc, reduced_high_cell_resistance, reduced_high_temp, reduced_low_cell_voltage, reduced_low_pack_voltage, reduced_voltage_failsafe, reduced_comms_failsafe);
    }

    double get_internal_cell_communication_fault() const { return bps_bps_error_internal_cell_communication_fault_decode(internal_cell_communication_fault); }

    void set_internal_cell_communication_fault(double value) { internal_cell_communication_fault = bps_bps_error_internal_cell_communication_fault_encode(value); }

    bool internal_cell_communication_fault_in_range() const { return bps_bps_error_internal_cell_communication_fault_is_in_range(internal_cell_communication_fault); }

    double get_weak_cell_fault() const { return bps_bps_error_weak_cell_fault_decode(weak_cell_fault); }

    void set_weak_cell_fault(double value) { weak_cell_fault = bps_bps_error_weak_cell_fault_encode(value); }

    bool weak_cell_fault_in_range() const { return bps_bps_error_weak_cell_fault_is_in_range(weak_cell_fault); }

    double get_low_cell_voltage_fault() const { return bps_bps_error_low_cell_voltage_fault_decode(low_cell_voltage_fault); }

    void set_low_cell_voltage_fault(double value) { low_cell_voltage_fault = bps_bps_error_low_cell_voltage_fault_encode(value); }

    bool low_cell_voltage_fault_in_range() const { return bps_bps_error_low_cell_voltage_fault_is_in_range(low_cell_voltage_fault); }

    double get_cell_open_wiring_fault() const { return bps_bps_error_cell_open_wiring_fault_decode(cell_open_wiring_fault); }

    void set_cell_open_wiring_fault(double value) { cell_open_wiring_fault = bps_bps_error_cell_open_wiring_fault_encode(value); }

    bool cell_open_wiring_fault_in_range() const { return bps_bps_error_cell_open_wiring_fault_is_in_range(cell_open_wiring_fault); }

    double get_thermistor_fault() const { return bps_bps_error_thermistor_fault_decode(thermistor_fault); }

    void set_thermistor_fault(double value) { thermistor_fault = bps_bps_error_thermistor_fault_encode(value); }

    bool thermistor_fault_in_range() const { return bps_bps_error_thermistor_fault_is_in_range(thermistor_fault); }

    double get_current_sensor_fault() const { return bps_bps_error_current_sensor_fault_decode(current_sensor_fault); }

    void set_current_sensor_fault(double value) { current_sensor_fault = bps_bps_error_current_sensor_fault_encode(value); }

    bool current_sensor_fault_in_range() const { return bps_bps_error_current_sensor_fault_is_in_range(current_sensor_fault); }

    double get_weak_pack_fault() const { return bps_bps_error_weak_pack_fault_decode(weak_pack_fault); }

    void set_weak_pack_fault(double value) { weak_pack_fault = bps_bps_error_weak_pack_fault_encode(value); }

    bool weak_pack_fault_in_range() const { return bps_bps_error_weak_pack_fault_is_in_range(weak_pack_fault); }

    double get_can_communication_fault() const { return bps_bps_error_can_communication_fault_decode(can_communication_fault); }

    void set_can_communication_fault(double value) { can_communication_fault = bps_bps_error_can_communication_fault_encode(value); }

    bool can_communication_fault_in_range() const { return bps_bps_error_can_communication_fault_is_in_range(can_communication_fault); }

    double get_redundant_power_supply_fault() const { return bps_bps_error_redundant_power_supply_fault_decode(redundant_power_supply_fault); }

    void set_redundant_power_supply_fault(double value) { redundant_power_supply_fault = bps_bps_error_redundant_power_supply_fault_encode(value); }

    bool redundant_power_supply_fault_in_range() const { return bps_bps_error_redundant_power_supply_fault_is_in_range(redundant_power_supply_fault); }

    double get_high_voltage_isolation_fault() const { return bps_bps_error_high_voltage_isolation_fault_decode(high_voltage_isolation_fault); }

    void set_high_voltage_isolation_fault(double value) { high_voltage_isolation_fault = bps_bps_error_high_voltage_isolation_fault_encode(value); }

    bool high_voltage_isolation_fault_in_range() const { return bps_bps_error_high_voltage_isolation_fault_is_in_range(high_voltage_isolation_fault); }

    double get_charge_enable_relay_fault() const { return bps_bps_error_charge_enable_relay_fault_decode(charge_enable_relay_fault); }

    void set_charge_enable_relay_fault(double value) { charge_enable_relay_fault = bps_bps_error_charge_enable_relay_fault_encode(value); }

    bool charge_enable_relay_fault_in_range() const { return bps_bps_error_charge_enable_relay_fault_is_in_range(charge_enable_relay_fault); }

    double get_discharge_enable_relay_fault() const { return bps_bps_error_discharge_enable_relay_fault_decode(discharge_enable_relay_fault); }

    void set_discharge_enable_relay_fault(double value) { discharge_enable_relay_fault = bps_bps_error_discharge_enable_relay_fault_encode(value); }

    bool discharge_enable_relay_fault_in_range() const { return bps_bps_error_discharge_enable_relay_fault_is_in_range(discharge_enable_relay_fault); }

    double get_internal_conversion_fault() const { return bps_bps_error_internal_conversion_fault_decode(internal_conversion_fault); }

    void set_internal_conversion_fault(double value) { internal_conversion_fault = bps_bps_error_internal_conversion_fault_encode(value); }

    bool internal_conversion_fault_in_range() const { return bps_bps_error_internal_conversion_fault_is_in_range(internal_conversion_fault); }

    double get_internal_memory_fault() const { return bps_bps_error_internal_memory_fault_decode(internal_memory_fault); }

    void set_internal_memory_fault(double value) { internal_memory_fault = bps_bps_error_internal_memory_fault_encode(value); }

    bool internal_memory_fault_in_range() const { return bps_bps_error_internal_memory_fault_is_in_range(internal_memory_fault); }

    double get_internal_thermistor_fault() const { return bps_bps_error_internal_thermistor_fault_decode(internal_thermistor_fault); }

    void set_internal_thermistor_fault(double value) { internal_thermistor_fault = bps_bps_error_internal_thermistor_fault_encode(value); }

    bool internal_thermistor_fault_in_range() const { return bps_bps_error_internal_thermistor_fault_is_in_range(internal_thermistor_fault); }

    double get_internal_logic_fault() const { return bps_bps_error_internal_logic_fault_decode(internal_logic_fault); }

    void set_internal_logic_fault(double value) { internal_logic_fault = bps_bps_error_internal_logic_fault_encode(value); }

    bool internal_logic_fault_in_range() const { return bps_bps_error_internal_logic_fault_is_in_range(internal_logic_fault); }

    double get_reduced_low_soc() const { return bps_bps_error_reduced_low_soc_decode(reduced_low_soc); }

    void set_reduced_low_soc(double value) { reduced_low_soc = bps_bps_error_reduced_low_soc_encode(value); }

    bool reduced_low_soc_in_range() const { return bps_bps_error_reduced_low_soc_is_in_range(reduced_low_soc); }

    double get_reduced_high_cell_resistance() const { return bps_bps_error_reduced_high_cell_resistance_decode(reduced_high_cell_resistance); }

    void set_reduced_high_cell_resistance(double value) { reduced_high_cell_resistance = bps_bps_error_reduced_high_cell_resistance_encode(value); }

    bool reduced_high_cell_resistance_in_range() const { return bps_bps_error_reduced_high_cell_resistance_is_in_range(reduced_high_cell_resistance); }

    double get_reduced_high_temp() const { return bps_bps_error_reduced_high_temp_decode(reduced_high_temp); }

    void set_reduced_high_temp(double value) { reduced_high_temp = bps_bps_error_reduced_high_temp_encode(value); }

    bool reduced_high_temp_in_range() const { return bps_bps_error_reduced_high_temp_is_in_range(reduced_high_temp); }

    double get_reduced_low_cell_voltage() const { return bps_bps_error_reduced_low_cell_voltage_decode(reduced_low_cell_voltage); }

    void set_reduced_low_cell_voltage(double value) { reduced_low_cell_voltage = bps_bps_error_reduced_low_cell_voltage_encode(value); }

    bool reduced_low_cell_voltage_in_range() const { return bps_bps_error_reduced_low_cell_voltage_is_in_range(reduced_low_cell_voltage); }

    double get_reduced_low_pack_voltage() const { return bps_bps_error_reduced_low_pack_voltage_decode(reduced_low_pack_voltage); }

    void set_reduced_low_pack_voltage(double value) { reduced_low_pack_voltage = bps_bps_error_reduced_low_pack_voltage_encode(value); }

    bool reduced_low_pack_voltage_in_range() const { return bps_bps_error_reduced_low_pack_voltage_is_in_range(reduced_low_pack_voltage); }

    double get_reduced_voltage_failsafe() const { return bps_bps_error_reduced_voltage_failsafe_decode(reduced_voltage_failsafe); }

    void set_reduced_voltage_failsafe(double value) { reduced_voltage_failsafe = bps_bps_error_reduced_voltage_failsafe_encode(value); }

    bool reduced_voltage_failsafe_in_range() const { return bps_bps_error_reduced_voltage_failsafe_is_in_range(reduced_voltage_failsafe); }

    double get_reduced_comms_failsafe() const { return bps_bps_error_reduced_comms_failsafe_decode(reduced_comms_failsafe); }

    void set_reduced_comms_failsafe(double value) { reduced_comms_failsafe = bps_bps_error_reduced_comms_failsafe_encode(value); }

    bool reduced_comms_failsafe_in_range() const { return bps_bps_error_reduced_comms_failsafe_is_in_range(reduced_comms_failsafe); }

    bool has_active_fault() {
        return internal_cell_communication_fault || weak_cell_fault || low_cell_voltage_fault || cell_open_wiring_fault || thermistor_fault || current_sensor_fault || weak_pack_fault || can_communication_fault || redundant_power_supply_fault || high_voltage_isolation_fault || charge_enable_relay_fault || discharge_enable_relay_fault || internal_conversion_fault || internal_memory_fault || internal_thermistor_fault || internal_logic_fault;
    }
} BpsError;

#endif