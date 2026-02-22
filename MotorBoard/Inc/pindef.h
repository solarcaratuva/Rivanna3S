#ifndef PINDEF_H
#define PINDEF_H

#define USB_TX          PC_12
#define USB_RX          PD_2

#define MAIN_CAN_STBY   PB_4
#define MAIN_CAN_RX     PB_5
#define MAIN_CAN_TX     PB_6

#define ID              PC_5

#define MTR_SDA         PA_8
#define MTR_SCL         PA_9
#define MTR_CAN_RX      PA_11
#define MTR_CAN_TX      PA_12

// Aliases used by MotorBoard/Src/main.cpp
#define LOG_TX          USB_TX
#define LOG_RX          USB_RX

#define CAN_STANDBY     MAIN_CAN_STBY
#define CAN_RX          MAIN_CAN_RX
#define CAN_TX          MAIN_CAN_TX

#define MOTOR_CAN_RX    MTR_CAN_RX
#define MOTOR_CAN_TX    MTR_CAN_TX

#endif
