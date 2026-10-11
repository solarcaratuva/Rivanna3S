#ifndef TELEMETRYBOARD_PINDEF_H
#define TELEMETRYBOARD_PINDEF_H

// Issue #204's pinout comment identifies PC1 as Debug_OUT on every STM board.
#define DEBUG_OUT PC_1
#define FAULT_LED_1_PORT GPIOC
#define FAULT_LED_1_PIN GPIO_PIN_1
#define FAULT_LED_1_ACTIVE_HIGH 1
// Active-high development-board defaults; verify the fitted LED wiring.
#if defined(STM32G474xx)
#define FAULT_LED_2_PORT GPIOB
#define FAULT_LED_2_PIN GPIO_PIN_9
#elif defined(STM32H743xx)
#define FAULT_LED_2_PORT GPIOB
#define FAULT_LED_2_PIN GPIO_PIN_0
#elif defined(STM32U5A9xx)
#define FAULT_LED_2_PORT GPIOE
#define FAULT_LED_2_PIN GPIO_PIN_0
#endif
#define FAULT_LED_2_ACTIVE_HIGH 1

#ifdef STM32G474xx

#define DEBUG_LED_1     PC_13
#define RADIO_DTR       PA_1
#define RADIO_TX        PA_2
#define RADIO_RX        PA_3
#define DEBUG_BTN       PA_4
#define SD_SELECT       PB_12
#define SPI2_SCK        PB_13 
#define SPI2_MISO       PB_14
#define SPI2_MOSI       PB_15
#define EEPROM_SELECT   PC_6
#define EEPROM_WRITE    PC_7
#define EEPROM_HOLD     PC_8
#define IMU_SDA         PA_8
#define IMU_SCL         PA_9
#define CAN_STBY        PA_10
#define CAN_RX          PA_11
#define CAN_TX          PA_12
#define LOG_TX          PC_10
#define LOG_RX          PC_11
#define GPS_TX          PC_12
#define GPS_RX          PD_2
#define SWO             PB_3
#define LTE_DTR         PB_5
#define LTE_TX          PB_6
#define LTE_RX          PB_7
#define DEBUG_LED_2     PB_9
#define BRAKE_PRESSURE  PC_4

#elif defined(STM32H743xx)

#define DEBUG_LED_1     NC
#define RADIO_DTR       NC
#define RADIO_TX        NC
#define RADIO_RX        NC
#define DEBUG_BTN       NC
#define SD_SELECT       NC
#define SPI2_SCK        NC
#define SPI2_MISO       NC
#define SPI2_MOSI       NC
#define EEPROM_SELECT   NC
#define EEPROM_WRITE    NC
#define EEPROM_HOLD     NC
#define IMU_SDA         NC
#define IMU_SCL         NC
#define CAN_STBY        NC
#define CAN_RX          NC
#define CAN_TX          NC
#define LOG_TX          PD_8 // ST-LINK virtual COM port (USART3)
#define LOG_RX          PD_9
#define GPS_TX          NC
#define GPS_RX          NC
#define SWO             NC
#define LTE_DTR         NC
#define LTE_TX          NC
#define LTE_RX          NC
#define DEBUG_LED_2     NC
#define BRAKE_PRESSURE  NC

#elif defined(STM32U5A9xx)
// Only fault LED outputs are mapped; other U5 application pins are undefined.

#else
#error "TelemetryBoard has no pin definitions for this target."
#endif

#endif /* TELEMETRYBOARD_PINDEF_H */
