#ifndef BOTTOMDISTBOARD_PINDEF_H
#define BOTTOMDISTBOARD_PINDEF_H

#define DEBUG_OUT PC_1
#define FAULT_LED_1_PORT GPIOC
#define FAULT_LED_1_PIN GPIO_PIN_1
#define FAULT_LED_1_ACTIVE_HIGH 1
// Active-high LED defaults; confirm the board wiring.
#if defined(STM32G474xx)
#define FAULT_LED_2_PORT GPIOA
#define FAULT_LED_2_PIN GPIO_PIN_5
#elif defined(STM32H743xx)
#define FAULT_LED_2_PORT GPIOB
#define FAULT_LED_2_PIN GPIO_PIN_0
#elif defined(STM32U5A9xx)
#define FAULT_LED_2_PORT GPIOE
#define FAULT_LED_2_PIN GPIO_PIN_0
#endif
#define FAULT_LED_2_ACTIVE_HIGH 1

#ifdef STM32G474xx

#define LEFT_TURN_EN PA_1
#define RIGHT_TURN_EN PA_0
// #define STROBE_EN NC
#define DRL_EN PA_2

#define THROTTLE_WIPER PA_3
#define BRAKE_WIPER PA_4

#define CAN_TX PB_6
#define CAN_RX PB_5
#define CAN_STANDBY PB_4

#define LOG_TX PC_12
#define LOG_RX PD_2

#elif defined(STM32H743xx)

#define LEFT_TURN_EN NC
#define RIGHT_TURN_EN NC
#define DRL_EN NC

#define THROTTLE_WIPER NC
#define BRAKE_WIPER NC

#define CAN_TX NC
#define CAN_RX NC
#define CAN_STANDBY NC

#define LOG_TX PD_8 // ST-LINK virtual COM port (USART3)
#define LOG_RX PD_9

#elif defined(STM32U5A9xx)
// Only fault LED outputs are mapped; other U5 application pins are undefined.

#else
#error "BottomDistBoard has no pin definitions for this target."
#endif

#endif /* BOTTOMDISTBOARD_PINDEF_H */
