#include "can.h"
#include <string.h> // memcpy
#include "pinmap.h"
#include "log.h"
#include "peripheralmap.h"
#include "lock.h"
#include "fdcan.h"
#include "Clock.h"
#include "DigitalOut.h"
#include "stm32_hal.h"
#include "FreeRTOS.h"
#include "task.h"

// extern "C" void HAL_FDCAN_MspInit_custom(FDCAN_GlobalTypeDef* fdcanHandle, Pin pin, uint8_t af);
// extern "C" FDCAN_HandleTypeDef* FDCAN_init(FDCAN_GlobalTypeDef* fdcan, uint32_t baudrate);

DigitalOut light(PC_1);

// The supported STM32 families expose at most three FDCAN peripherals.
static CAN* can_instance_map[3] = {};


CAN::CAN(Pin tx, Pin rx, uint32_t baudrate)
   
{
    configASSERT(FDCAN_PERIPHERAL_COUNT <= 3);
    fdcan_periph = findCANPin(tx, rx);
    if (fdcan_periph == nullptr) {
        initialized = false;
        log_warn("CAN init failed: no matching peripheral for TX/RX pins");
        return;
    }
    fdcan_periph->rxd_used = rx;
    fdcan_periph->txd_used = tx;

    gpio_clock_enable(tx.block);
    gpio_clock_enable(rx.block);
    
    uint8_t tx_af = get_FDCAN_AF(fdcan_periph->handle, &tx, TX);
    uint8_t rx_af = get_FDCAN_AF(fdcan_periph->handle, &rx, RX);
    HAL_FDCAN_MspInit_custom(fdcan_periph->handle, tx, tx_af);
    HAL_FDCAN_MspInit_custom(fdcan_periph->handle, rx, rx_af);

    hfdcan = FDCAN_init(fdcan_periph->handle, baudrate);
    if (!hfdcan || hfdcan->Instance != fdcan_periph->handle ||
        HAL_FDCAN_GetState(hfdcan) != HAL_FDCAN_STATE_READY) {
        log_warn("CAN init failed: HAL initialization failed");
        return;
    }

    IRQn_Type rx_irq = FDCAN1_IT0_IRQn;
#if defined(FDCAN2)
    if (hfdcan->Instance == FDCAN2) rx_irq = FDCAN2_IT0_IRQn;
#endif
#if defined(FDCAN3)
    if (hfdcan->Instance == FDCAN3) rx_irq = FDCAN3_IT0_IRQn;
#endif
    const uint32_t rx_notifications = FDCAN_IT_RX_FIFO0_NEW_MESSAGE |
                                      FDCAN_IT_RX_FIFO0_FULL |
                                      FDCAN_IT_RX_FIFO0_MESSAGE_LOST;
#if defined(STM32H743xx)
    const uint32_t rx_interrupts = rx_notifications;
#else
    const uint32_t rx_interrupts = FDCAN_IT_GROUP_RX_FIFO0;
#endif
    if (HAL_FDCAN_ConfigInterruptLines(hfdcan, rx_interrupts, FDCAN_INTERRUPT_LINE0) != HAL_OK ||
        HAL_FDCAN_Start(hfdcan) != HAL_OK) {
        log_warn("CAN init failed: interrupt routing or start failed");
        return;
    }

    for (size_t i = 0; i < FDCAN_PERIPHERAL_COUNT; ++i) {
        if (FDCAN_Peripherals[i].handle == hfdcan->Instance) {
            can_instance_map[i] = this;
            break;
        }
    }
    HAL_NVIC_SetPriority(rx_irq, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_ClearPendingIRQ(rx_irq);
    if (HAL_FDCAN_ActivateNotification(hfdcan, rx_notifications, 0) != HAL_OK) {
        HAL_FDCAN_Stop(hfdcan);
        for (size_t i = 0; i < FDCAN_PERIPHERAL_COUNT; ++i) {
            if (can_instance_map[i] == this) can_instance_map[i] = nullptr;
        }
        log_warn("CAN init failed: RX notification setup failed");
        return;
    }
    HAL_NVIC_EnableIRQ(rx_irq);

    // Default Tx header setup; fields that change per-frame will be set in write().
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0;

    initialized = true;
}


int CAN::write(const SerializedCanMessage *msg)
{
    if (!initialized) {
        log_warn("CAN write failed: CAN not initialized");
        return 1;
    }

    instance_lock.lock();
    txHeader.Identifier = msg->id;
    txHeader.DataLength = bytesToDlc(msg->len);

    // HAL expects a uint8_t* to data
    HAL_StatusTypeDef status =
        HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader,
                                      const_cast<uint8_t *>(msg->data));

    if (status != HAL_OK) {
        log_warn("CAN write failed: status %d, error code %lx", status, hfdcan->ErrorCode);
        instance_lock.unlock();
        return 2;
    }

    instance_lock.unlock();
    return 0;
}


int CAN::write(CanMessage *msg)
{
    SerializedCanMessage scm;
    msg->serialize(&scm);

    return write(&scm);
}


int CAN::read(SerializedCanMessage *msg)
{
    if (!initialized) {
        log_warn("CAN read failed: CAN not initialized");
        return 1;
    }

    // Only one reader may own the FIFO/waiter; writers use a separate lock.
    rx_lock.lock();
    for (;;) {
        taskENTER_CRITICAL();
        if (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) != 0) {
            rxTask = nullptr;
            taskEXIT_CRITICAL();
            break;
        }
        rxTask = xTaskGetCurrentTaskHandle();
        taskEXIT_CRITICAL();

        // A notification arriving before this call is retained by FreeRTOS.
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        taskENTER_CRITICAL();
        rxTask = nullptr;
        taskEXIT_CRITICAL();
    }

    instance_lock.lock();

    uint8_t rxData[8] = {0};

    HAL_StatusTypeDef status =
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData);

    if (status != HAL_OK)
    {
        log_warn("CAN read failed: status %d, error code %lx", status, hfdcan->ErrorCode);
        instance_lock.unlock();
        rx_lock.unlock();
        return 2;
    }

    msg->id = static_cast<uint16_t>(rxHeader.Identifier);
    msg->len = dlcToBytes(rxHeader.DataLength);
    memcpy(msg->data, rxData, msg->len);
    instance_lock.unlock();
    rx_lock.unlock();

    bool message_lost;
    taskENTER_CRITICAL();
    message_lost = rx_message_lost;
    rx_message_lost = false;
    taskEXIT_CRITICAL();

    if (message_lost) {
        log_warn("CAN RX FIFO0 message lost");
    }

    return 0;
}

int CAN::try_read(SerializedCanMessage *msg)
{
    if (!initialized) {
        log_warn("CAN read failed: CAN not initialized");
        return 1;
    }

    if (!rx_lock.try_lock()) {
        return 3;
    }

    uint32_t pending = HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0);
    if (pending == 0) {
        rx_lock.unlock();
        return 3;
    }

    instance_lock.lock();

    uint8_t rxData[8] = {0};

    HAL_StatusTypeDef status =
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData);

    if (status != HAL_OK)
    {
        log_warn("CAN read failed: status %d, error code %lx", status, hfdcan->ErrorCode);
        instance_lock.unlock();
        rx_lock.unlock();
        return 2;
    }

    msg->id = static_cast<uint16_t>(rxHeader.Identifier);
    msg->len = dlcToBytes(rxHeader.DataLength);
    memcpy(msg->data, rxData, msg->len);

    instance_lock.unlock();
    rx_lock.unlock();

    return 0;
}


// ********* Helper functions *********

uint32_t CAN::bytesToDlc(uint8_t len) const
{
    // Clamp to 8 bytes; extend if you move to CAN FD later.
    if (len > 8) {
        len = 8;
    }

    switch (len) {
    case 0: return FDCAN_DLC_BYTES_0;
    case 1: return FDCAN_DLC_BYTES_1;
    case 2: return FDCAN_DLC_BYTES_2;
    case 3: return FDCAN_DLC_BYTES_3;
    case 4: return FDCAN_DLC_BYTES_4;
    case 5: return FDCAN_DLC_BYTES_5;
    case 6: return FDCAN_DLC_BYTES_6;
    case 7: return FDCAN_DLC_BYTES_7;
    case 8: return FDCAN_DLC_BYTES_8;
    default: return FDCAN_DLC_BYTES_8;
    }
}

uint8_t CAN::dlcToBytes(uint32_t dlc) const
{
    switch (dlc) {
    case FDCAN_DLC_BYTES_0: return 0;
    case FDCAN_DLC_BYTES_1: return 1;
    case FDCAN_DLC_BYTES_2: return 2;
    case FDCAN_DLC_BYTES_3: return 3;
    case FDCAN_DLC_BYTES_4: return 4;
    case FDCAN_DLC_BYTES_5: return 5;
    case FDCAN_DLC_BYTES_6: return 6;
    case FDCAN_DLC_BYTES_7: return 7;
    case FDCAN_DLC_BYTES_8: return 8;
    default: return 8; // conservative fallback
    }
}


FDCAN_Peripheral *CAN::findCANPin(Pin tx, Pin rx)
{
    for (uint8_t i = 0; i < FDCAN_PERIPHERAL_COUNT; i++)
    {
        FDCAN_Peripheral *peripheral = &FDCAN_Peripherals[i];
        if (((*peripheral).txd_valid_pins & tx.universal_mask) &&
            ((*peripheral).rxd_valid_pins & rx.universal_mask))
        {
            if (!peripheral->isClaimed)
            {
                peripheral->isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching ADC peripheral found
}

CAN* CAN::find_from_handle(FDCAN_HandleTypeDef* handle)
{
    if (!handle) return nullptr;
    for (size_t i = 0; i < FDCAN_PERIPHERAL_COUNT; ++i) {
        if (FDCAN_Peripherals[i].handle == handle->Instance) {
            return can_instance_map[i];
        }
    }
    return nullptr;
}

void CAN::notify_rx_from_isr(uint32_t interrupts)
{
    BaseType_t woken = pdFALSE;
    if (interrupts & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) {
        rx_message_lost = true;
    }
    if (rxTask) {
        vTaskNotifyGiveFromISR(rxTask, &woken);
        rxTask = nullptr;
    }
    portYIELD_FROM_ISR(woken);
}

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* handle, uint32_t interrupts)
{
    const uint32_t rx_interrupts = FDCAN_IT_RX_FIFO0_NEW_MESSAGE |
                                   FDCAN_IT_RX_FIFO0_FULL |
                                   FDCAN_IT_RX_FIFO0_MESSAGE_LOST;
    if (interrupts & rx_interrupts) {
        CAN* can = CAN::find_from_handle(handle);
        if (can) can->notify_rx_from_isr(interrupts);
    }
}
