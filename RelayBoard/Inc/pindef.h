#ifndef RELAYBOARD_PINDEF_H
#define RELAYBOARD_PINDEF_H

#ifdef TARGET_STM32G474RET6

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

#else
#error "RelayBoard has no pin definitions for this target; expected TARGET_STM32G474RET6."
#endif

#endif /* RELAYBOARD_PINDEF_H */
