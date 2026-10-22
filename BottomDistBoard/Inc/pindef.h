#ifndef BOTTOMDISTBOARD_PINDEF_H
#define BOTTOMDISTBOARD_PINDEF_H

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

#define DEBUG_IN PC_13
#define DEBUG_OUT PC_1
#define ID PC_5

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

#define DEBUG_IN NC
#define DEBUG_OUT NC
#define ID NC

#else
#error "BottomDistBoard has no pin definitions for this target."
#endif

#endif /* BOTTOMDISTBOARD_PINDEF_H */
