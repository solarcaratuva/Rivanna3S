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
#include "Precharge.h"
#include "Rivanna3SCanStructs.h"
#include "log.h"
#include "pindef.h"
#include "thread.h"
/* USER CODE END Includes */

uint32_t PRECHARGE_CONTROL_PERIOD_MS = 100;

bool has_other_fault = false;
bool has_cont12_fault = false;
uint16_t pack_voltage = 0;

DigitalOut motor_main_en(MAIN_EN);
DigitalOut motor_precharge_en(PRECHARGE_EN);
DigitalOut mppt_main_en(MPPT_PWR_ON);
DigitalOut mppt_precharge_en(PRECHARGE_MPPT_EN);
AnalogIn cont12_voltage(CONT12_VOLTAGE);
AnalogIn motor_hal_effect_voltage(HAL_EFFECT_VOLTAGE);
AnalogIn mppt_hal_effect_voltage(HAL_EFFECT_MPPT);

CanInterface main_can(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);
Thread precharge_thread;

Precharge motor_precharge(motor_main_en, motor_precharge_en, cont12_voltage, motor_hal_effect_voltage);
Precharge mppt_precharge(mppt_main_en, mppt_precharge_en, cont12_voltage, mppt_hal_effect_voltage);

void handle_bps_status(const SerializedCanMessage &msg)
{
    BpsStatus status{};
    status.deserialize(&msg);
    pack_voltage = status.pack_voltage;
}

void handle_bps_fault(const SerializedCanMessage &msg)
{
    BpsError status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_other_fault = true;
        log_fault("BPS fault detected!");
    }
}

void handle_contactor_fault(const SerializedCanMessage &msg)
{
    Contactor12Error status{};
    status.deserialize(&msg);

    if (status.has_active_fault())
    {
        has_cont12_fault = true;
        log_fault("Contactor12 fault detected!");
    }
}

void handle_motor_fault(const SerializedCanMessage &msg)
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
        motor_precharge.run(pack_voltage, has_cont12_fault, has_other_fault);
        mppt_precharge.run(pack_voltage, has_cont12_fault, has_other_fault);

        PrechargeStatus status{};
        status.motor_stage = motor_precharge.stage();
        status.mppt_stage = mppt_precharge.stage();
        status.cont12_fault = has_cont12_fault ? 1 : 0;
        status.other_fault = has_other_fault ? 1 : 0;
        status.threshold = motor_precharge.threshold();
        status.cont12 = motor_precharge.cont12_high() ? 1 : 0;
        status.hal_effect_motor = motor_precharge.hal_effect_millivolts();
        status.hal_effect_mppt = mppt_precharge.hal_effect_millivolts();

        main_can.write(&status);
        precharge_clock.sleep_since(PRECHARGE_CONTROL_PERIOD_MS);
    }
}

void app_main()
{
    log_configure(INFO_LVL, LOG_TX, LOG_RX, 921600);
    log_info("Relay Board starting up...");

    main_can.register_callback(BpsStatus::get_message_ID(), handle_bps_status);
    main_can.register_callback(BpsError::get_message_ID(), handle_bps_fault);
    main_can.register_callback(Contactor12Error::get_message_ID(), handle_contactor_fault);
    main_can.register_callback(MotorControllerError::get_message_ID(), handle_motor_fault);

    precharge_thread.start(run_precharge);

    log_info("Relay Board initialized");
    Clock::sleep_forever();
}
