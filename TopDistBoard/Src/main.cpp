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

DigitalOut left_turn_signal(LEFT_TURN_EN);
DigitalOut right_turn_signal(RIGHT_TURN_EN);
DigitalOut bps_strobe(BMS_STROBE_EN);
DigitalOut brake_light(BRAKE_EN);

bool flashLeftTurnSignal = false;
bool flashRightTurnSignal = false;
bool flashHazards = false;
bool has_faulted = false;
bool brake_from_pedal = false;
bool brake_from_motor = false;
bool bms_fault_active = false;

#define SIGNAL_FLASH_PERIOD 500

CanInterface main_can(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);

Thread signal_thread;

void handle_dashboard_commands(const SerializedCanMessage &msg)
{
    DashboardCommands cmd{};
    cmd.deserialize(&msg);

    flashHazards = cmd.hazards;
    flashLeftTurnSignal = cmd.left_turn_signal;
    flashRightTurnSignal = cmd.right_turn_signal;
}

void handle_pedal_status(const SerializedCanMessage &msg)
{
    PedalStatus status{};
    status.deserialize(&msg);

    brake_from_pedal = status.brake_pedal;
    brake_light.write(brake_from_pedal || brake_from_motor);
}

void handle_motor_commands(const SerializedCanMessage &msg)
{
    MotorCommands commands{};
    commands.deserialize(&msg);

    brake_from_motor = commands.regen_braking > 0;
    brake_light.write(brake_from_pedal || brake_from_motor);
}

void handle_bpsfault_messages(const SerializedCanMessage &msg)
{
    BpsError status{};
    status.deserialize(&msg);
    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("BPS fault detected!");
    }
    bms_fault_active = status.has_active_fault();
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

void handle_motor_controller_fault(const SerializedCanMessage &msg)
{
    MotorControllerError status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("Motor controller fault detected!");
    }
}

void signal_flash_handler()
{
    Clock signal_flash_clock;

    while (true) {
        if (bms_fault_active) {
            bps_strobe.write(!bps_strobe.read());
        } else {
            bps_strobe.write(PIN_OFF);
        }

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

void log_missed_heartbeat() {
    log_fault("missed heartbeat callback func xxxx");
}

void app_main()
{
    log_configure(DEBUG_LVL, LOG_TX, LOG_RX, 921600);
    log_info("Top Dist Board starting up...");

    HeartbeatSafetySystem::setup(&main_can, log_missed_heartbeat, Node::TopDistBoard);

    signal_thread.start(signal_flash_handler);

    main_can.register_callback(DashboardCommands::get_message_ID(), handle_dashboard_commands);
    main_can.register_callback(PedalStatus::get_message_ID(), handle_pedal_status);
    main_can.register_callback(MotorCommands::get_message_ID(), handle_motor_commands);
    main_can.register_callback(BpsError::get_message_ID(), handle_bpsfault_messages);
    main_can.register_callback(Contactor12Error::get_message_ID(), handle_contactor_fault);
    main_can.register_callback(MotorControllerError::get_message_ID(), handle_motor_controller_fault);

    log_info("Top Dist Board initialized");
    Clock::sleep_forever();
}
