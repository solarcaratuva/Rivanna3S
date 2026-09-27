#ifndef TELEMETRYBOARD_PINDEF_H
#define TELEMETRYBOARD_PINDEF_H

#ifdef TARGET_STM32G474RET6

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

#else
#error "TelemetryBoard has no pin definitions for this target; expected TARGET_STM32G474RET6."
#endif

#endif /* TELEMETRYBOARD_PINDEF_H */
