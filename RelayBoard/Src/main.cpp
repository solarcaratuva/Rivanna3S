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
#include "Precharge.h"
/* USER CODE END Includes */

uint32_t PRECHARGE_CONTROL_PERIOD_MS = 100;

 bool has_other_fault = false;
 bool has_cont12_fault = false;
 uint16_t pack_voltage = 0;

DigitalOut main_en(MAIN_EN);
DigitalOut precharge_en(PRECHARGE_EN);
AnalogIn cont12_voltage(CONT12_VOLTAGE);
AnalogIn hal_effect_voltage(HAL_EFFECT_VOLTAGE);

CanInterface main_can(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);
Thread precharge_thread;

Precharge precharge(main_en, precharge_en, cont12_voltage, hal_effect_voltage);

void handle_bps_status(SerializedCanMessage &msg)
{
    BpsStatus status{};
    status.deserialize(&msg);
    pack_voltage = status.pack_voltage;
}

void handle_bps_fault(SerializedCanMessage &msg)
{
    BpsError status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_other_fault = true;
        log_fault("BPS fault detected!");
    }
}

void handle_contactor_fault(SerializedCanMessage &msg)
{
    Contactor12Error status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_cont12_fault = true;
        log_fault("Contactor12 fault detected!");
    }
}

void handle_motor_fault(SerializedCanMessage &msg)
{
    MotorControllerError status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_other_fault = true;
        log_fault("Motor controller fault detected!");
    }
}

void run_precharge()
{
    Clock precharge_clock;

    while (true)
    {
        precharge.run(pack_voltage, has_cont12_fault, has_other_fault);
        main_can.write(&precharge.status());

        precharge_clock.sleep_since(PRECHARGE_CONTROL_PERIOD_MS);
    }
}


void app_main()
{
    log_configure(INFO_LVL, LOG_TX, LOG_RX, 250000);
    log_info("Relay Board starting up...");

    main_can.register_callback(BpsStatus::get_message_ID(), handle_bps_status);
    main_can.register_callback(BpsError::get_message_ID(), handle_bps_fault);
    main_can.register_callback(Contactor12Error::get_message_ID(), handle_contactor_fault);
    main_can.register_callback(MotorControllerError::get_message_ID(), handle_motor_fault);

    precharge_thread.start(run_precharge);

    log_info("Relay Board initialized");
}
