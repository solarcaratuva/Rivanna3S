#ifndef PINDEF_H
#define PINDEF_H

#define USB_TX          PC_12
#define USB_RX          PD_2

#define MAIN_CAN_STBY   PB_4
#define MAIN_CAN_RX     PB_5
#define MAIN_CAN_TX     PB_6

#define DEBUG_IN        PC_13
#define DEBUG_OUT       PC_1

// PC1 is confirmed by issue #204's board pinout comment. PC13 is Debug_IN.
#define FAULT_LED_1_PORT GPIOC
#define FAULT_LED_1_PIN GPIO_PIN_1
#define FAULT_LED_1_ACTIVE_HIGH 1
// Best-guess active-high LED defaults from the matching development boards.
// Verify or change these outputs for the fitted car-board hardware.
#if defined(STM32H743xx)
#define FAULT_LED_2_PORT GPIOB
#define FAULT_LED_2_PIN GPIO_PIN_0
#elif defined(STM32U5A9xx)
#define FAULT_LED_2_PORT GPIOE
#define FAULT_LED_2_PIN GPIO_PIN_0
#else
#define FAULT_LED_2_PORT GPIOA
#define FAULT_LED_2_PIN GPIO_PIN_5
#endif
#define FAULT_LED_2_ACTIVE_HIGH 1

#define ID              PC_5

#define MTR_SDA         PA_8
#define MTR_SCL         PA_9
#define MTR_CAN_RX      PA_11
#define MTR_CAN_TX      PA_12

#define LOG_TX          USB_TX
#define LOG_RX          USB_RX

#define CAN_STANDBY     MAIN_CAN_STBY
#define CAN_RX          MAIN_CAN_RX
#define CAN_TX          MAIN_CAN_TX

#define MOTOR_CAN_RX    MTR_CAN_RX
#define MOTOR_CAN_TX    MTR_CAN_TX
#define MOTOR_CAN_STANDBY NC


// #define USB_TX          NC
// #define USB_RX          NC

// #define MAIN_CAN_STBY   NC
// #define MAIN_CAN_RX     NC
// #define MAIN_CAN_TX     NC

// #define DEBUG_IN        NC
// #define DEBUG_OUT       NC

// #define ID              NC

// #define MTR_SDA         NC
// #define MTR_SCL         NC
// #define MTR_CAN_RX      NC
// #define MTR_CAN_TX      NC

// #define LOG_TX          NC
// #define LOG_RX          NC

// #define CAN_STANDBY     NC
// #define CAN_RX          NC
// #define CAN_TX          NC

// #define MOTOR_CAN_RX    NC
// #define MOTOR_CAN_TX    NC
// #define MOTOR_CAN_STANDBY NC

#endif
