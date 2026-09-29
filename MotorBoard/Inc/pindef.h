#ifndef MOTORBOARD_PINDEF_H
#define MOTORBOARD_PINDEF_H

#ifdef TARGET_STM32G474RET6

#define USB_TX          PC_12
#define USB_RX          PD_2

#define MAIN_CAN_STBY   PB_4
#define MAIN_CAN_RX     PB_5
#define MAIN_CAN_TX     PB_6

#define DEBUG_IN        PC_13
#define DEBUG_OUT       PC_1

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

#elif defined(TARGET_STM32H743ZITX)

#define USB_TX          NC
#define USB_RX          NC

#define MAIN_CAN_STBY   NC
#define MAIN_CAN_RX     NC
#define MAIN_CAN_TX     NC

#define DEBUG_IN        NC
#define DEBUG_OUT       NC

#define ID              NC

#define MTR_SDA         NC
#define MTR_SCL         NC
#define MTR_CAN_RX      NC
#define MTR_CAN_TX      NC

#define LOG_TX          PD_8 // ST-LINK virtual COM port (USART3)
#define LOG_RX          PD_9

#define CAN_STANDBY     NC
#define CAN_RX          NC
#define CAN_TX          NC

#define MOTOR_CAN_RX    NC
#define MOTOR_CAN_TX    NC
#define MOTOR_CAN_STANDBY NC

#else
#error "MotorBoard has no pin definitions for this target; expected TARGET_STM32G474RET6 or TARGET_STM32H743ZITX."
#endif

#endif /* MOTORBOARD_PINDEF_H */
