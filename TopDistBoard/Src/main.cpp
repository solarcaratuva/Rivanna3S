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
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "AnalogIn.h"
#include "Clock.h"
#include "thread.h"
#include "log.h"
#include "Rivanna3SCanStructs.h"
#include "MotorControllerCanStructs.h"
#include "BPSCanStructs.h"
#include "CanInterface.h"
#include "pindef.h"

DigitalOut left_turn_signal(LEFT_TURN_EN);
DigitalOut right_turn_signal(RIGHT_TURN_EN);
DigitalOut bps_strobe(BMS_STROBE_EN);
DigitalOut brake_light(BRAKE_EN);

bool flashLeftTurnSignal = false;
bool flashRightTurnSignal = false;
bool flashHazards = false;
bool has_faulted = false; // true if there is any fault that locks the car until reset
bool brake_from_pedal = false;
bool brake_from_motor = false;
bool bms_strobe_on = false;

const bool PIN_ON = true;
const bool PIN_OFF = false;

#define LOG_LEVEL LOG_DEBUG
#define SIGNAL_FLASH_PERIOD 500

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);

Thread signal_thread;

// listen for DashboardCommands CAN message for turn signals and hazards
 void handle_dashboard_commands(SerializedCanMessage &msg)
{
    DashboardCommands cmd{};
    cmd.deserialize(&msg);

    flashHazards = cmd.hazards;
    flashLeftTurnSignal = cmd.left_turn_signal;
    flashRightTurnSignal = cmd.right_turn_signal;
    }


// listen for PedalStatus and MotorCommands CAN message to set brake lights
void handle_pedal_status(SerializedCanMessage &msg)
{
    PedalStatus status{};
    status.deserialize(&msg);

    brake_from_pedal = status.brake_pedal;
    brake_light.write(brake_from_pedal || brake_from_motor);
}

void handle_motor_commands(SerializedCanMessage &msg)
{
    MotorCommands commands{};
    commands.deserialize(&msg);

    brake_from_motor = commands.regen_braking > 0;
    brake_light.write(brake_from_pedal || brake_from_motor);
}

// listen for BpsError to set BPS strobe light
//Ask colby what this BPS strobe light is?
 void handle_bpsfault_messages(SerializedCanMessage &msg)
{
    BpsError status{};
    status.deserialize(&msg);
    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("BPS fault detected!");
    }
    bms_strobe_on = (status.has_active_fault() ? PIN_ON : PIN_OFF);
}

// listen for all faults to set hazards
void handle_contactor_fault(SerializedCanMessage &msg)
{
    Contactor12Error status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("Contactor fault detected!");
    }
}

void handle_motor_controller_fault(SerializedCanMessage &msg)
{
    MotorControllerError status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("Motor controller fault detected!");
    }
}


// turn signal flash handler
void signal_flash_handler()
{
    Clock signal_flash_clock;
    
     while (true) {
        if (bms_strobe_on) {
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

void app_main()
{

    log_configure(INFO_LVL, LOG_TX, LOG_RX, 250000);
    log_info("Top Dist Board starting up...");

    signal_thread.start(signal_flash_handler);

    main_can.register_callback(DashboardCommands::get_message_ID(), handle_dashboard_commands);
    main_can.register_callback(PedalStatus::get_message_ID(), handle_pedal_status);
    main_can.register_callback(MotorCommands::get_message_ID(), handle_motor_commands);
    main_can.register_callback(BpsError::get_message_ID(), handle_bpsfault_messages);
    main_can.register_callback(Contactor12Error::get_message_ID(), handle_contactor_fault);
    main_can.register_callback(MotorControllerError::get_message_ID(), handle_motor_controller_fault);

    log_info("Top Dist Board initialized");
}
