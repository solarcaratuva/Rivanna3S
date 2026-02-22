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
#include "I2C.h"
#include "MotorInterface.h"

AnalogIn throttle_pedal(THROTTLE_WIPER);
DigitalIn brake_pedal(BRAKE_WIPER);

bool has_faulted = false; // true if there is any fault that locks the car until reset
bool regen_enabled = false; // true if the brake is pressed and regen is enabled, false if the brake is pressed and regen is disabled or the brake is not pressed
uint16_t throttle = 0;
bool brake = false;

const bool PIN_ON = true;
const bool PIN_OFF = false;

#define LOG_LEVEL LOG_DEBUG
#define MOTOR_CONTROL_PERIOD 100

PedalStatus pedal_CAN_struct = PedalStatus();
MotorCommands motor_CAN_struct = MotorCommands();
I2C motor_control_serial_bus(MTR_SDA, MTR_SCL);
MotorInterface motor_interface(&motor_control_serial_bus);

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, CAN_STANDBY, 250000, CanNetwork::Main);
//CanInterface motor_can = CanInterface(MOTOR_CAN_TX, MOTOR_CAN_RX, MOTOR_CAN_STANDBY, 250000, CanNetwork::Motor);

Thread motor_control_thread;

// Motor Board
// motor control loop, send MotorCommands message every iteration

// listen for all faults to disable the motor
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


// listen for DashboardCommands message to control the motor control loop
void handle_dashboard_commands(SerializedCanMessage &msg)
{
    DashboardCommands cmd{};
    cmd.deserialize(&msg);
    regen_enabled = cmd.regen_en;

}

void handle_pedal_status(SerializedCanMessage &msg)
{
    pedal_CAN_struct.deserialize(&msg);
    throttle = pedal_CAN_struct.throttle_pedal;
    brake = pedal_CAN_struct.brake_pedal;
}




//helper functions from last years code

/**
 * Sets the throttle and regen values of the motor based on the regen and throttle formula
 */
void regen_drive(PedalStatus *pedal_CAN_struct, uint16_t* throttleValue, uint16_t* regenValue) {
    uint16_t pedalValue = *throttleValue;

    if (pedalValue <= 50) {
        *throttleValue = 0;
        *regenValue = 79.159 * pow(50 - pedalValue, 0.3);
        pedal_CAN_struct->brake_pedal = true;
    } else if (pedalValue < 100) {
        *throttleValue = 0;
        *regenValue = 0;
    } else {
        *throttleValue = -56.27610464 * pow(156 - (pedalValue - 100), 0.3) + 256;
        *regenValue = 0;
    }
}




/**
 * Function that polls the throttle and brake pedals and sets throttle and regen values
 * Checks if the system has faulted, breaks are enabled, cruise control is enabled, or regen is 
 * enabled and sets the throttle and regen values accordingly
 */
void set_motor_status() {

  Clock motor_control_clock;
  while(true){
    
    
    uint16_t regen = 0;
    pedal_CAN_struct.throttle_pedal = throttle;
    pedal_CAN_struct.brake_pedal = brake;

    if (has_faulted) { // fault case
        throttle = 0;
        regen = 0;
    } else if (brake) { // brake case
        throttle = 0;
        if (regen_enabled) {
            regen = 256; // max regen
        } else {
            regen = 0;
        }
        //cruise_control_brake_latch = true;
        pedal_CAN_struct.brake_pedal = true;
    // } else if (cruise_control_enabled && !cruise_control_brake_latch) { // cruise control case, logic handled elsewhere
    //     // TODO: get throttle
    //     motor_CAN_struct.cruise_drive = true;
    //     motor_CAN_struct.cruise_speed = motor_CAN_struct.cruise_drive ? cruise_control.get_cruise_target() : 0;
    } else if(regen_enabled) { // regen drive case
        regen_drive(&pedal_CAN_struct, &throttle, &regen);
        motor_CAN_struct.regen_braking = regen;
        if (regen > 0)
            pedal_CAN_struct.brake_pedal = true;
        motor_CAN_struct.regen_drive = true;
    } else { // normal drive case
        // do nothing; throttle, brake, and regen are already set
        motor_CAN_struct.manual_drive = true;
    }

    motor_interface.sendThrottle(throttle);
    motor_interface.sendRegen(regen);

    pedal_CAN_struct.throttle_pedal = throttle;
    main_can.write(&pedal_CAN_struct);
    motor_control_clock.sleep_since(MOTOR_CONTROL_PERIOD);
  }
}


void handle_motor_faults(SerializedCanMessage &msg) {
    MotorControllerError status{};
    status.deserialize(&msg);
    if (status.has_active_fault()) {
        has_faulted = true;
        log_fault("Motor controller fault detected!");
    }
}



// Message_forwarder is called whenever the MotorControllerCANInterface gets a CAN message.
// This forwards the message to the vehicle can bus.
void MotorControllerCANInterface::message_forwarder(CANMessage *message) {
    main_can.send_message(message);
}

void MotorControllerCANInterface::handle(MotorControllerPowerStatus *can_struct) {
    uint16_t motor_rpm = can_struct->motor_rpm;
    cruise_control.send_cruise_control_to_motor(motor_rpm);
}

void MotorControllerCANInterface::handle(MotorControllerDriveStatus *can_struct) {
    // we don't currently do anything with this CAN message
}





void app_main()
{
    log_configure(INFO_LVL, LOG_TX, LOG_RX, 250000);
    log_info("Motor Board starting up...");

    motor_control_thread.start(set_motor_status());

    main_can.register_callback(BpsError::get_message_ID(), handle_bpsfault_messages);
    main_can.register_callback(DashboardCommands::get_message_ID(), handle_dashboard_commands);
    main_can.register_callback(PedalStatus::get_message_ID(), handle_pedal_status);
    main_can.register_callback(MotorControllerError::get_message_ID(), handle_motor_faults);

    log_info("Motor Board initialized");
}
