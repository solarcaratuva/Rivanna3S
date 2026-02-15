
#ifndef mppt_CAN_Structs
#define mppt_CAN_Structs

#include "can.h"
#include "mppt.h"
#include "log.h"

typedef struct MG0Status : CanMessage, mppt_mg0_status_t {
    void serialize(SerializedCanMessage *message) {
        mppt_mg0_status_pack(message->data, this,
            MPPT_MG0_STATUS_LENGTH);
        message->len = MPPT_MG0_STATUS_LENGTH;
        message->id = MPPT_MG0_STATUS_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        mppt_mg0_status_unpack(this, message->data,
            MPPT_MG0_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return MPPT_MG0_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MG0Status: "
            );
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MG0Status;


typedef struct MG1Status : CanMessage, mppt_mg1_status_t {
    void serialize(SerializedCanMessage *message) {
        mppt_mg1_status_pack(message->data, this,
            MPPT_MG1_STATUS_LENGTH);
        message->len = MPPT_MG1_STATUS_LENGTH;
        message->id = MPPT_MG1_STATUS_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        mppt_mg1_status_unpack(this, message->data,
            MPPT_MG1_STATUS_LENGTH);
    }

    static uint16_t get_message_ID() { return MPPT_MG1_STATUS_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MG1Status: "
            );
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MG1Status;


typedef struct MG0OutputVoltageInputPower : CanMessage, mppt_mg0_output_voltage_input_power_t {
    void serialize(SerializedCanMessage *message) {
        mppt_mg0_output_voltage_input_power_pack(message->data, this,
            MPPT_MG0_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH);
        message->len = MPPT_MG0_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH;
        message->id = MPPT_MG0_OUTPUT_VOLTAGE_INPUT_POWER_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        mppt_mg0_output_voltage_input_power_unpack(this, message->data,
            MPPT_MG0_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH);
    }

    static uint16_t get_message_ID() { return MPPT_MG0_OUTPUT_VOLTAGE_INPUT_POWER_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MG0OutputVoltageInputPower: "
            );
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MG0OutputVoltageInputPower;


typedef struct MG1OutputVoltageInputPower : CanMessage, mppt_mg1_output_voltage_input_power_t {
    void serialize(SerializedCanMessage *message) {
        mppt_mg1_output_voltage_input_power_pack(message->data, this,
            MPPT_MG1_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH);
        message->len = MPPT_MG1_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH;
        message->id = MPPT_MG1_OUTPUT_VOLTAGE_INPUT_POWER_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        mppt_mg1_output_voltage_input_power_unpack(this, message->data,
            MPPT_MG1_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH);
    }

    static uint16_t get_message_ID() { return MPPT_MG1_OUTPUT_VOLTAGE_INPUT_POWER_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MG1OutputVoltageInputPower: "
            );
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MG1OutputVoltageInputPower;


typedef struct MG0PCBMOSFETTemperature : CanMessage, mppt_mg0_pcbmosfet_temperature_t {
    void serialize(SerializedCanMessage *message) {
        mppt_mg0_pcbmosfet_temperature_pack(message->data, this,
            MPPT_MG0_PCBMOSFET_TEMPERATURE_LENGTH);
        message->len = MPPT_MG0_PCBMOSFET_TEMPERATURE_LENGTH;
        message->id = MPPT_MG0_PCBMOSFET_TEMPERATURE_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        mppt_mg0_pcbmosfet_temperature_unpack(this, message->data,
            MPPT_MG0_PCBMOSFET_TEMPERATURE_LENGTH);
    }

    static uint16_t get_message_ID() { return MPPT_MG0_PCBMOSFET_TEMPERATURE_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MG0PCBMOSFETTemperature: MG0PCBTemperature %u, MG0MOSFETTemperature %u",
            mg0_pcb_temperature, mg0_mosfet_temperature);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MG0PCBMOSFETTemperature;


typedef struct MG1PCBMOSFETTemperature : CanMessage, mppt_mg1_pcbmosfet_temperature_t {
    void serialize(SerializedCanMessage *message) {
        mppt_mg1_pcbmosfet_temperature_pack(message->data, this,
            MPPT_MG1_PCBMOSFET_TEMPERATURE_LENGTH);
        message->len = MPPT_MG1_PCBMOSFET_TEMPERATURE_LENGTH;
        message->id = MPPT_MG1_PCBMOSFET_TEMPERATURE_FRAME_ID;
    }

    void deserialize(SerializedCanMessage *message) {
        mppt_mg1_pcbmosfet_temperature_unpack(this, message->data,
            MPPT_MG1_PCBMOSFET_TEMPERATURE_LENGTH);
    }

    static uint16_t get_message_ID() { return MPPT_MG1_PCBMOSFET_TEMPERATURE_FRAME_ID; }

    void log_msg(LogLevel level) const {
        log(level, __FILE__, __LINE__,
            "MG1PCBMOSFETTemperature: MG1PCBTemperature %u, MG1MOSFETTemperature %u",
            mg1_pcb_temperature, mg1_mosfet_temperature);
    }

    bool has_active_fault() {
        return 0; // this message has no fault signals
    }
} MG1PCBMOSFETTemperature;

#endif