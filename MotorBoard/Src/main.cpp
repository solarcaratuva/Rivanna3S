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
#include <cmath>

#include "AnalogIn.h"
#include "BPSCanStructs.h"
#include "CanInterface.h"
#include "Clock.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "../../Common/Drivers/Inc/I2C.h"
#include "MotorControllerCanStructs.h"
#include "MotorInterface.h"
#include "Rivanna3SCanStructs.h"
#include "log.h"
#include "pindef.h"
#include "thread.h"

bool has_faulted = false;
bool regen_enabled = false;
uint16_t throttle = 0;
bool brake = false;

#define MOTOR_CONTROL_PERIOD 100

I2C motor_control_serial_bus(MTR_SDA, MTR_SCL, I2C::STANDARD);
MotorInterface motor_interface(&motor_control_serial_bus);

CanInterface main_can(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);
CanInterface motor_can(MOTOR_CAN_TX, MOTOR_CAN_RX, MOTOR_CAN_STANDBY, 250000, CanNetwork::Motor);

Thread motor_control_thread;

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

void handle_dashboard_commands(const SerializedCanMessage &msg)
{
    DashboardCommands cmd{};
    cmd.deserialize(&msg);
    regen_enabled = cmd.regen_en;
}

void handle_pedal_status(const SerializedCanMessage &msg)
{
    PedalStatus pedal_can_struct{};
    pedal_can_struct.deserialize(&msg);
    throttle = pedal_can_struct.throttle_pedal;
    brake = pedal_can_struct.brake_pedal;
}

void regen_drive(uint16_t *throttle_value, uint16_t *regen_value)
{
    uint16_t pedal_value = *throttle_value;

    if (pedal_value <= 50)
    {
        *throttle_value = 0;
        *regen_value = 79.159 * std::pow(50 - pedal_value, 0.3);
    }
    else if (pedal_value < 100)
    {
        *throttle_value = 0;
        *regen_value = 0;
    }
    else
    {
        *throttle_value = -56.27610464 * std::pow(156 - (pedal_value - 100), 0.3) + 256;
        *regen_value = 0;
    }
}

void set_motor_status()
{
    MotorCommands motor_can_struct{};
    Clock motor_control_clock;

    while (true)
    {
        uint16_t current_throttle = throttle;
        uint16_t regen = 0;
        motor_can_struct = MotorCommands();

        if (has_faulted)
        {
            current_throttle = 0;
            regen = 0;
        }
        else if (brake)
        {
            current_throttle = 0;
            if (regen_enabled)
            {
                regen = 256;
            }
        }
        else if (regen_enabled)
        {
            regen_drive(&current_throttle, &regen);
            motor_can_struct.regen_drive = true;
        }
        else
        {
            motor_can_struct.manual_drive = true;
        }

        motor_interface.sendThrottle(current_throttle);
        motor_interface.sendRegen(regen);

        motor_can_struct.throttle = current_throttle;
        motor_can_struct.regen_braking = regen;
        main_can.write(&motor_can_struct);
        motor_control_clock.sleep_since(MOTOR_CONTROL_PERIOD);
    }
}

void handle_motor_faults(const SerializedCanMessage &msg)
{
    MotorControllerError status{};
    status.deserialize(&msg);
    if (status.has_active_fault())
    {
        has_faulted = true;
        log_fault("Motor controller fault detected!");
    }
}

void forward_motor_can_message(const SerializedCanMessage &msg)
{
    main_can.write(&msg);
}

void app_main()
{
    log_configure(INFO_LVL, LOG_TX, LOG_RX, 921600);
    log_info("Motor Board starting up...");

    motor_control_thread.start(set_motor_status);

    main_can.register_callback(BpsError::get_message_ID(), handle_bpsfault_messages);
    main_can.register_callback(DashboardCommands::get_message_ID(), handle_dashboard_commands);
    main_can.register_callback(PedalStatus::get_message_ID(), handle_pedal_status);
    main_can.register_callback(Contactor12Error::get_message_ID(), handle_contactor_fault);
    motor_can.register_always_callback(forward_motor_can_message);
    motor_can.register_callback(MotorControllerError::get_message_ID(), handle_motor_faults);

    log_info("Motor Board initialized");
    Clock::sleep_forever();
}
