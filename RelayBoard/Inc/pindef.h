#ifndef RELAYBOARD_PINDEF_H
#define RELAYBOARD_PINDEF_H

#define DEBUG_OUT          PC_1

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

#define LOG_TX             PC_12
#define LOG_RX             PD_2
#define CAN_STANDBY        PB_4
#define CAN_RX             PB_5
#define CAN_TX             PB_6
#define SAFETY_HV_EN       PA_1
#define SAFETY_MTR_EN      PA_2
#define AUX_PLUS           PB_11
#define CONT12_VOLTAGE     PB_12
#define HAL_EFFECT_MPPT    PB_13
#define HAL_EFFECT_VOLTAGE PB_14
#define MAIN_EN            PB_15
#define PRECHARGE_EN       PC_6
#define MPPT_PWR_ON        PC_7
#define PRECHARGE_MPPT_EN  PC_8

// #define LOG_TX             NC
// #define LOG_RX             NC
// #define CAN_STANDBY        NC
// #define CAN_RX             NC
// #define CAN_TX             NC
// #define SAFETY_HV_EN       NC
// #define SAFETY_MTR_EN      NC
// #define AUX_PLUS           NC
// #define CONT12_VOLTAGE     NC
// #define HAL_EFFECT_MPPT    NC
// #define HAL_EFFECT_VOLTAGE NC
// #define MAIN_EN            NC
// #define PRECHARGE_EN       NC
// #define MPPT_PWR_ON        NC
// #define PRECHARGE_MPPT_EN  NC

#endif
