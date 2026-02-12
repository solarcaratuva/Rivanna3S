/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "UART.h"
#include "AnalogIn.h"
#include "Timeout.h"
#include "Clock.h"
#include "thread.h"
#include "lock.h"
#include "log.h"

#include "Rivanna3SCanStructs.h"
#include "MotorControllerCanStructs.h"
#include "MPPTCanStructs.h"
#include "BPSCanStructs.h"
#include "bps.h"
#include "CanInterface.h"
#include "pindef.h"
/* USER CODE END Includes */


DigitalOut left_turn_signal(LEFT_TURN_EN);
DigitalOut right_turn_signal(RIGHT_TURN_EN);
DigitalOut drl(DRL_EN);
DigitalOut hazard_signal(HAZARD_EN);
DigitalOut bms_strobe(STROBE_EN);

AnalogIn throttle_pedal(THROTTLE_WIPER);
DigitalIn brake_pedal(BRAKE_WIPER);

bool flashLeftTurnSignal = false;
bool flashRightTurnSignal = false;
bool flashHazards = false;
bool faultHazards = false;
bool bms_error = false;
bool drl_enabled = true;

const bool PIN_ON = true;
const bool PIN_OFF = false;

#define LOG_LEVEL LOG_DEBUG
#define SIGNAL_FLASH_PERIOD 500ms
#define BRAKE_LIGHTS_UPDATE_PERIOD 10ms
#define MOTOR_CONTROL_PERIOD 10ms
#define MOTOR_REQUEST_FRAMES_PERIOD 10ms
#define AUX_BATTERY_PERIOD 1s
#define MAX_REGEN 256



CanInterface *main_can;

Thread signal_thread;
Thread pedal_thread;
Thread motor_control_thread;
Thread motor_request_frames_thread;


static bool bps_error_has_fault(const bps_bps_error_t &err)
{
    return err.internal_cell_communication_fault ||
           err.weak_cell_fault ||
           err.low_cell_voltage_fault ||
           err.cell_open_wiring_fault ||
           err.current_sensor_fault ||
           err.weak_pack_fault ||
           err.thermistor_fault ||
           err.can_communication_fault ||
           err.redundant_power_supply_fault ||
           err.high_voltage_isolation_fault ||
           err.charge_enable_relay_fault ||
           err.discharge_enable_relay_fault ||
           err.internal_hardware_fault ||
           err.internal_heatsink_thermistor_fault ||
           err.internal_logic_fault ||
           err.highest_cell_voltage_too_high_fault ||
           err.lowest_cell_voltage_too_low_fault ||
           err.pack_too_hot_fault;
}

 void handle_fault_messages(const SerializedCanMessage &msg)
{
    if (msg.id != BPS_BPS_ERROR_FRAME_ID)
    {
        return;
    }

    BpsError status{};
    SerializedCanMessage copy = msg;
    status.deserialize(&copy);
    if (!status.has_active_fault())
    {
        return;
    }

    bms_error = fault;
    faultHazards = fault;
}

 void handle_dashboard_commands(const SerializedCanMessage &msg)
{
    DashboardCommands cmd{};
    SerializedCanMessage copy = msg;
    cmd.deserialize(&copy);

    flashHazards = cmd.hazards;
    flashLeftTurnSignal = cmd.left_turn_signal;
    flashRightTurnSignal = cmd.right_turn_signal;

    drl.write(drl_enabled ? PIN_ON : PIN_OFF);
}

 void signal_flash_handler()
{
    static bool flash_phase = false;
    flash_phase = !flash_phase;

    const bool hazards_active = flashHazards || faultHazards;
    bool left_on = false;
    bool right_on = false;
    bool hazard_on = false;

    if (hazards_active)
    {
        left_on = flash_phase;
        right_on = flash_phase;
        hazard_on = flash_phase;
    }
    else if (flashLeftTurnSignal)
    {
        left_on = flash_phase;
    }
    else if (flashRightTurnSignal)
    {
        right_on = flash_phase;
    }

    left_turn_signal.write(left_on ? PIN_ON : PIN_OFF);
    right_turn_signal.write(right_on ? PIN_ON : PIN_OFF);
    hazard_signal.write(hazard_on ? PIN_ON : PIN_OFF);
    bms_strobe.write((bms_error && flash_phase) ? PIN_ON : PIN_OFF);
}

void send_pedal_status()
{
    const uint16_t throttle = throttle_pedal.read_u12();

    MotorCommands msg{};
 
    msg.throttle = throttle;

    main_can->write(&msg);
}

void signal_flash_task()
{

    while (1)
    {
        signal_flash_handler();
    }
}

void pedal_status_task()
{

    while (1)
    {
        send_pedal_status();
    }
}


extern "C" void app_main(void *argument)
{
    (void)argument;

    log_configure(INFO_LVL, PD_8, PD_9, 250000);

    left_turn_signal.write(PIN_OFF);
    right_turn_signal.write(PIN_OFF);
    hazard_signal.write(PIN_OFF);
    bms_strobe.write(PIN_OFF);
    drl.write(drl_enabled ? PIN_ON : PIN_OFF);

    static CanInterface can(CAN_TX, CAN_RX, BAUDRATE, CanNetwork::Main);
    main_can = &can;
    can.register_callback(DashboardCommands::get_message_ID(), handle_dashboard_commands);
    can.register_always_callback(handle_fault_messages);

    signal_thread.start(signal_flash_task);
    pedal_thread.start(pedal_status_task);

}
