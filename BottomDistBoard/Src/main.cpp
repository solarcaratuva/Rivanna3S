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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "AnalogIn.h"
#include "BPSCanStructs.h"
#include "CanInterface.h"
#include "Clock.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "MotorControllerCanStructs.h"
#include "Rivanna3SCanStructs.h"
#include "log.h"
#include "pindef.h"
#include "thread.h"
#include "heartbeat.h"
/* USER CODE END Includes */

DigitalOut left_turn_signal(LEFT_TURN_EN);
DigitalOut right_turn_signal(RIGHT_TURN_EN);
DigitalOut drl(DRL_EN);
AnalogIn throttle_pedal(THROTTLE_WIPER);
//DigitalIn brake_pedal(BRAKE_WIPER);
AnalogIn brake_pedal(BRAKE_WIPER);

bool flashLeftTurnSignal = false;
bool flashRightTurnSignal = false;
bool flashHazards = false;
bool has_faulted = false;

#define SIGNAL_FLASH_PERIOD 500
#define PEDAL_STATUS 100
#define THROTTLE_VOLTAGE_LOW 0.82
#define THROTTLE_VOLTAGE_HIGH 3.3
#define THROTTLR_VOLTAGE_DIFFERENCE (THROTTLE_VOLTAGE_HIGH - THROTTLE_VOLTAGE_LOW)

CanInterface main_can(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);

Thread signal_thread;
Thread pedal_thread;

void handle_bpsfault_messages(const SerializedCanMessage &msg)
{
    BpsError status{};
    status.deserialize(&msg);
    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("BPS fault detected!");
    }
}

void handle_contactor_fault(const SerializedCanMessage &msg)
{
    Contactor12Error status{};
    status.deserialize(&msg);
    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("Contactor fault detected!");
    }
}

void handle_motor_fault(const SerializedCanMessage &msg)
{
    MotorControllerError status{};
    status.deserialize(&msg);
    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("Motor controller fault detected!");
    }
}

void handle_dashboard_commands(const SerializedCanMessage &msg)
{
    DashboardCommands cmd{};
    cmd.deserialize(&msg);

    flashHazards = cmd.hazards;
    flashLeftTurnSignal = cmd.left_turn_signal;
    flashRightTurnSignal = cmd.right_turn_signal;

    drl.write(cmd.charging_mode_en ? PIN_OFF : PIN_ON);
}

void signal_flash_handler()
{
    Clock signal_flash_clock;

    while (true) {
        if (flashHazards || has_faulted) {
            left_turn_signal.write(!left_turn_signal.read());
            right_turn_signal.write(left_turn_signal.read());
        } else if (flashLeftTurnSignal) {
            left_turn_signal.write(!left_turn_signal.read());
            right_turn_signal.write(PIN_OFF);
        } else if (flashRightTurnSignal) {
            right_turn_signal.write(!right_turn_signal.read());
            left_turn_signal.write(PIN_OFF);
        } else {
            left_turn_signal.write(PIN_OFF);
            right_turn_signal.write(PIN_OFF);
        }

        signal_flash_clock.sleep_since(SIGNAL_FLASH_PERIOD);
    }
}

uint16_t read_throttle()
{
    float adjusted_throttle_input =
        (throttle_pedal.read_voltage() - THROTTLE_VOLTAGE_LOW) / THROTTLR_VOLTAGE_DIFFERENCE;
    if (adjusted_throttle_input <= 0.0f) {
        return 0;
    } else if (adjusted_throttle_input >= 1.0f) {
        return 256;
    } else {
        return static_cast<uint16_t>(adjusted_throttle_input * 256.0);
    }
}

void send_pedal_status()
{
    Clock pedal_status_clock;

    while (true) {
        const uint16_t current_throttle = read_throttle();

        PedalStatus msg{};
        msg.throttle_pedal = current_throttle;
        msg.brake_pedal = brake_pedal.read_voltage() > 1.0f ? 1:0; //brake pedal is analog signal;

        main_can.write(&msg);
        pedal_status_clock.sleep_since(PEDAL_STATUS);
    }
}

void missed_heartbeat_callback() {
    log_fault("missed heartbeat callback func xxxx");
}

void app_main()
{   
    DigitalOut init_indicator(PC_1);
    init_indicator.write(PIN_ON);

    log_configure(INFO_LVL, LOG_TX, LOG_RX, 921600);
    log_info("Bottom Distance Board starting up...");

    // HeartbeatSafetySystem::setup(&main_can, missed_heartbeat_callback, Node::BottomDistBoard);

    drl.write(PIN_ON);

    signal_thread.start(signal_flash_handler);
    pedal_thread.start(send_pedal_status);

    main_can.register_callback(DashboardCommands::get_message_ID(), handle_dashboard_commands);
    main_can.register_callback(BpsError::get_message_ID(), handle_bpsfault_messages);
    main_can.register_callback(Contactor12Error::get_message_ID(), handle_contactor_fault);
    main_can.register_callback(MotorControllerError::get_message_ID(), handle_motor_fault);

    log_info("Bottom Distance Board initialized");
    Clock::sleep_forever();
}
