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
/* USER CODE END Includes */


DigitalOut left_turn_signal(LEFT_TURN_EN);
DigitalOut right_turn_signal(RIGHT_TURN_EN);
DigitalOut drl(DRL_EN);
DigitalOut hazard_signal(HAZARD_EN);
AnalogIn throttle_pedal(THROTTLE_WIPER);
DigitalIn brake_pedal(BRAKE_WIPER);

bool flashLeftTurnSignal = false;
bool flashRightTurnSignal = false;
bool flashHazards = false;
bool has_faulted = false; // true if there is any fault that locks the car until reset

const bool PIN_ON = true;
const bool PIN_OFF = false;

#define LOG_LEVEL LOG_DEBUG
#define SIGNAL_FLASH_PERIOD 500
#define PEDAL_STATUS 100

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, 250000, CanNetwork::Main);

Thread signal_thread;
Thread pedal_thread;


 void handle_bpsfault_messages(SerializedCanMessage &msg)
{
    BpsError status{};
    status.deserialize(&msg);
    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("BPS fault detected!");
    }
}

 void handle_dashboard_commands(SerializedCanMessage &msg)
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
    
    while (1) {
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

void send_pedal_status()
{
    Clock pedal_status_clock;
    
    while (1) {
        const uint16_t throttle = throttle_pedal.read_u12();

        PedalStatus msg{};
 
        msg.throttle_pedal = throttle;
        msg.brake_pedal = brake_pedal.read();

        main_can.write(&msg);

        pedal_status_clock.sleep_since(PEDAL_STATUS);
    }   
}


extern "C" void app_main(void *argument)
{
    (void)argument;

    log_configure(INFO_LVL, LOG_TX, LOG_RX, 250000);
    log_info("Bottom Distance Board starting up...");

    drl.write(PIN_ON); // DRL is on by default, turned off when charging mode is enabled

    signal_thread.start(signal_flash_handler);
    pedal_thread.start(send_pedal_status);

    main_can.register_callback(BpsError::get_message_ID(), handle_bpsfault_messages);
    main_can.register_callback(Contactor12Error::get_message_ID(), handle_dashboard_commands);
    main_can.register_callback(MotorControllerError::get_message_ID(), handle_dashboard_commands);

    log_info("Bottom Distance Board initialized");
}
