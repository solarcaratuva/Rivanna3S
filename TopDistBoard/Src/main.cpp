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
#include "FaultHandler.h"

DigitalOut left_turn_signal(LEFT_TURN_EN);
DigitalOut right_turn_signal(RIGHT_TURN_EN);
DigitalOut bps_strobe(BMS_STROBE_EN);
DigitalOut brake_light(BRAKE_EN);

bool flashLeftTurnSignal = false;
bool flashRightTurnSignal = false;
bool flashHazards = false;
bool brake_from_pedal = false;
bool brake_from_motor = false;

#define SIGNAL_FLASH_PERIOD 500
#define SIGNAL_FAULT_PERIOD 250

CanInterface main_can(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);

Thread signal_thread;
Thread fault_thread;

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

void signal_flash_handler()
{
    Clock signal_flash_clock;

    while (true) {

        if (flashHazards || FaultHandler::has_any_fault()) {
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

void bms_strobe_handler() {
    Clock signal_fault_clock;

    while (true){
        if (FaultHandler::has_bps_fault() || FaultHandler::has_contactor_fault()) {
            bps_strobe.write(!bps_strobe.read());
        } else {
            bps_strobe.write(PIN_OFF);
        }
        signal_fault_clock.sleep_since(SIGNAL_FAULT_PERIOD);
    }
}

void app_main()
{
    log_configure(DEBUG_LVL, LOG_TX, LOG_RX, 921600);
    log_info("Top Dist Board starting up...");

    signal_thread.start(signal_flash_handler);
    fault_thread.start(bms_strobe_handler);

    main_can.register_callback(DashboardCommands::get_message_ID(), handle_dashboard_commands);
    main_can.register_callback(PedalStatus::get_message_ID(), handle_pedal_status);
    main_can.register_callback(MotorCommands::get_message_ID(), handle_motor_commands);
    main_can.register_always_callback(FaultHandler::check_for_any_faults);
    log_info("Top Dist Board initialized");
    Clock::sleep_forever();
}
