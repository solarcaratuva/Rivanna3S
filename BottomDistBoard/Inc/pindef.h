#ifndef BOTTOMDISTBOARD_PINDEF_H
#define BOTTOMDISTBOARD_PINDEF_H

#ifdef STM32G474xx

#define LEFT_TURN_EN PA_1
#define RIGHT_TURN_EN PA_0
// #define STROBE_EN NC
// TEMP (Nucleo bring-up): PA_2/PA_3 are reassigned to the ST-LINK virtual COM
// port so monitor.py can read the log. On NUCLEO-G474RE those pins reach the
// ST-LINK through solder bridges SB17/SB23. DRL_EN and THROTTLE_WIPER are
// parked on NC for now; restore them to PA_2/PA_3 for the real PCB.
#define DRL_EN NC

#define THROTTLE_WIPER NC
#define BRAKE_WIPER PA_4

#define CAN_TX PB_6
#define CAN_RX PB_5
#define CAN_STANDBY PB_4

#define LOG_TX PA_2
#define LOG_RX PA_3

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

#else
#error "BottomDistBoard has no pin definitions for this target."
#endif

#endif /* BOTTOMDISTBOARD_PINDEF_H */
