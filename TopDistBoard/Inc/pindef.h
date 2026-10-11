#define DEBUG_OUT PC_1

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

#define LEFT_TURN_EN PA_1
#define RIGHT_TURN_EN PA_0
#define BMS_STROBE_EN PC_2

#define BRAKE_EN PC_3

#define CAN_TX PB_6
#define CAN_RX PB_5
#define CAN_STANDBY PB_4

#define LOG_TX PC_12
#define LOG_RX PD_2


// #define LEFT_TURN_EN NC
// #define RIGHT_TURN_EN NC
// #define BMS_STROBE_EN NC

// #define BRAKE_EN NC

// #define CAN_TX NC
// #define CAN_RX NC
// #define CAN_STANDBY NC

// #define LOG_TX NC
// #define LOG_RX NC
