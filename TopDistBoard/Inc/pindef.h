#ifndef TOPDISTBOARD_PINDEF_H
#define TOPDISTBOARD_PINDEF_H

#ifdef TARGET_STM32G474RET6

#define LEFT_TURN_EN PA_1
#define RIGHT_TURN_EN PA_0
#define BMS_STROBE_EN PC_2

#define BRAKE_EN PC_3

#define CAN_TX PB_6
#define CAN_RX PB_5
#define CAN_STANDBY PB_4

#define LOG_TX PC_12
#define LOG_RX PD_2

#elif defined(TARGET_STM32H743ZITX)

#define LEFT_TURN_EN NC
#define RIGHT_TURN_EN NC
#define BMS_STROBE_EN NC

#define BRAKE_EN NC

#define CAN_TX NC
#define CAN_RX NC
#define CAN_STANDBY NC

#define LOG_TX PD_8 // ST-LINK virtual COM port (USART3)
#define LOG_RX PD_9

#else
#error "TopDistBoard has no pin definitions for this target; expected TARGET_STM32G474RET6 or TARGET_STM32H743ZITX."
#endif

#endif /* TOPDISTBOARD_PINDEF_H */
