#ifndef NUCLEOBOARD_PINDEF_H
#define NUCLEOBOARD_PINDEF_H

#ifdef STM32G474xx

/* CAN bench target -- NUCLEO-G474RE. Not a car board; see Src/main.cpp.
 *
 * FDCAN2, the same pins the car's main bus uses on RelayBoard, TopDistBoard,
 * BottomDistBoard and MotorBoard's main_can. Keeping the instance identical is
 * the point: FDCAN2 is the one the NVIC priority bug affected, and FDCAN1 is
 * the one configuration that always worked.
 */
#define CAN_RX          PB_5
#define CAN_TX          PB_6
/* No transceiver standby control on a breakout; tie the transceiver's Rs to
 * ground instead. CanInterface drives this low at startup, so leaving it NC
 * costs nothing. */
#define CAN_STANDBY     NC

/* Second CAN instance, for the two-interface tests. FDCAN1 on PA_11/PA_12,
 * which is what MotorBoard's motor_can uses. Wire a second transceiver here
 * and put it on the SAME CANH/CANL pair as the first: MotorBoard is the only
 * board running two instances at once, so can_instance_map and the
 * per-instance NVIC priority have no other way to be tested, and one shared
 * bus needs only one adapter.
 *
 * Only built when DUAL_INSTANCE is set in main.cpp. Leaving it off is the
 * default because an FDCAN1 with no transceiver sees a floating RX, which
 * reads dominant and parks that instance in bus off. */
#define AUX_CAN_RX      PA_11
#define AUX_CAN_TX      PA_12
#define AUX_CAN_STANDBY NC

/* USART2. PA_2/PA_3 is where a Nucleo-64 wires its ST-Link virtual COM port,
 * so the log comes out of the same USB cable that flashes the board. The car
 * boards use PC_12/PD_2 or PC_10/PC_11, neither of which is connected to
 * anything on a Nucleo. */
#define LOG_TX          PA_2
#define LOG_RX          PA_3

#else
#error "NucleoBoard only has pin definitions for the STM32G474."
#endif

#endif /* NUCLEOBOARD_PINDEF_H */
