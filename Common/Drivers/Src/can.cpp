#include "can.h"
#include <string.h> // memcpy
#include "pinmap.h"
#include "log.h"
#include "peripheralmap.h"
#include "lock.h"
#include "fdcan.h"
#include "stm32_hal.h"
#include "FreeRTOS.h"
#include "task.h"

// Maps an FDCAN peripheral back to its CAN object, so the HAL's interrupt
// callback can find us. Indexed the same way as FDCAN_Peripherals.
static CAN *can_instance_map[3] = {};

static CAN *can_for_handle(FDCAN_HandleTypeDef *handle)
{
    if (!handle)
    {
        return nullptr;
    }

    for (size_t i = 0; i < FDCAN_PERIPHERAL_COUNT; ++i)
    {
        if (FDCAN_Peripherals[i].handle == handle->Instance)
        {
            return can_instance_map[i];
        }
    }

    return nullptr;
}

CAN::CAN(Pin tx, Pin rx, uint32_t baudrate)
{
    configASSERT(FDCAN_PERIPHERAL_COUNT <= 3);

    fdcan_periph = findCANPin(tx, rx);
    if (fdcan_periph == nullptr)
    {
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
    if (!hfdcan || hfdcan->Instance != fdcan_periph->handle || HAL_FDCAN_GetState(hfdcan) != HAL_FDCAN_STATE_READY)
    {
        log_warn("CAN init failed: HAL initialization failed");
        return;
    }

    IRQn_Type rx_irq = FDCAN1_IT0_IRQn;
#if defined(FDCAN2)
    if (hfdcan->Instance == FDCAN2)
    {
        rx_irq = FDCAN2_IT0_IRQn;
    }
#endif
#if defined(FDCAN3)
    if (hfdcan->Instance == FDCAN3)
    {
        rx_irq = FDCAN3_IT0_IRQn;
    }
#endif

    // The G4 HAL takes interrupt group constants here, the H7 takes individual
    // flags.
#if defined(STM32H743xx)
    const uint32_t rx_interrupts = FDCAN_IT_RX_FIFO0_NEW_MESSAGE;
#else
    const uint32_t rx_interrupts = FDCAN_IT_GROUP_RX_FIFO0;
#endif

    HAL_StatusTypeDef route_status = HAL_FDCAN_ConfigInterruptLines(hfdcan, rx_interrupts, FDCAN_INTERRUPT_LINE0);
    if (route_status != HAL_OK)
    {
        log_warn("CAN init failed: interrupt routing, status %d", route_status);
        return;
    }

    HAL_StatusTypeDef start_status = HAL_FDCAN_Start(hfdcan);
    if (start_status != HAL_OK)
    {
        log_warn("CAN init failed: start, status %d", start_status);
        return;
    }

    // Register before enabling the IRQ, so the handler can never run before
    // there is an instance for it to find.
    for (size_t i = 0; i < FDCAN_PERIPHERAL_COUNT; ++i)
    {
        if (FDCAN_Peripherals[i].handle == hfdcan->Instance)
        {
            can_instance_map[i] = this;
            break;
        }
    }

    HAL_NVIC_ClearPendingIRQ(rx_irq);

    const uint32_t rx_notifications = FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO0_MESSAGE_LOST;
    HAL_StatusTypeDef notify_status = HAL_FDCAN_ActivateNotification(hfdcan, rx_notifications, 0);
    if (notify_status != HAL_OK)
    {
        HAL_FDCAN_Stop(hfdcan);
        for (size_t i = 0; i < FDCAN_PERIPHERAL_COUNT; ++i)
        {
            if (can_instance_map[i] == this)
            {
                can_instance_map[i] = nullptr;
            }
        }
        log_warn("CAN init failed: RX notification, status %d", notify_status);
        return;
    }

    // Must be no higher priority than configMAX_SYSCALL_INTERRUPT_PRIORITY, or
    // vTaskNotifyGiveFromISR() in the handler trips configASSERT. Set it before
    // enabling, so it cannot fire at the wrong priority even once.
    HAL_NVIC_SetPriority(rx_irq, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(rx_irq);

    // Fields that change per frame are set in write().
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
    if (!initialized)
    {
        log_warn("CAN write failed: CAN not initialized");
        return 1;
    }

    instance_lock.lock();
    txHeader.Identifier = msg->id;
    txHeader.DataLength = bytesToDlc(msg->len);

    HAL_StatusTypeDef status = HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader, const_cast<uint8_t *>(msg->data));
    if (status != HAL_OK)
    {
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
    if (!initialized)
    {
        log_warn("CAN read failed: CAN not initialized");
        return 1;
    }

    rx_lock.lock();

    // Publish rxTask before checking the FIFO. If a frame arrives in between,
    // the handler has already seen the task and its notification is latched, so
    // ulTaskNotifyTake() returns rather than sleeping on a frame that is
    // already here.
    while (true)
    {
        rxTask = xTaskGetCurrentTaskHandle();

        uint32_t pending = HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0);
        if (pending != 0)
        {
            break;
        }

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
    rxTask = nullptr;

    instance_lock.lock();

    uint8_t rxData[8] = {0};
    HAL_StatusTypeDef status = HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData);
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

uint32_t CAN::get_rx_lost_count() const
{
    return rx_lost_count;
}

void CAN::handle_rx_interrupt_from_isr(uint32_t interrupts)
{
    if (interrupts & FDCAN_IT_RX_FIFO0_MESSAGE_LOST)
    {
        ++rx_lost_count;
    }

    if (!(interrupts & FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
    {
        return;
    }

    BaseType_t woken = pdFALSE;
    if (rxTask)
    {
        vTaskNotifyGiveFromISR(rxTask, &woken);
        rxTask = nullptr;
    }
    portYIELD_FROM_ISR(woken);
}

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *handle, uint32_t interrupts)
{
    CAN *can = can_for_handle(handle);
    if (can)
    {
        can->handle_rx_interrupt_from_isr(interrupts);
    }
}

// ********* Helper functions *********

uint32_t CAN::bytesToDlc(uint8_t len) const
{
    if (len > 8)
    {
        len = 8;
    }

    switch (len)
    {
    case 0:
        return FDCAN_DLC_BYTES_0;
    case 1:
        return FDCAN_DLC_BYTES_1;
    case 2:
        return FDCAN_DLC_BYTES_2;
    case 3:
        return FDCAN_DLC_BYTES_3;
    case 4:
        return FDCAN_DLC_BYTES_4;
    case 5:
        return FDCAN_DLC_BYTES_5;
    case 6:
        return FDCAN_DLC_BYTES_6;
    case 7:
        return FDCAN_DLC_BYTES_7;
    case 8:
        return FDCAN_DLC_BYTES_8;
    default:
        return FDCAN_DLC_BYTES_8;
    }
}

uint8_t CAN::dlcToBytes(uint32_t dlc) const
{
    switch (dlc)
    {
    case FDCAN_DLC_BYTES_0:
        return 0;
    case FDCAN_DLC_BYTES_1:
        return 1;
    case FDCAN_DLC_BYTES_2:
        return 2;
    case FDCAN_DLC_BYTES_3:
        return 3;
    case FDCAN_DLC_BYTES_4:
        return 4;
    case FDCAN_DLC_BYTES_5:
        return 5;
    case FDCAN_DLC_BYTES_6:
        return 6;
    case FDCAN_DLC_BYTES_7:
        return 7;
    case FDCAN_DLC_BYTES_8:
        return 8;
    default:
        return 8;
    }
}

FDCAN_Peripheral *CAN::findCANPin(Pin tx, Pin rx)
{
    for (uint8_t i = 0; i < FDCAN_PERIPHERAL_COUNT; i++)
    {
        FDCAN_Peripheral *peripheral = &FDCAN_Peripherals[i];

        bool pins_match = (peripheral->txd_valid_pins & tx.universal_mask) && (peripheral->rxd_valid_pins & rx.universal_mask);
        if (pins_match && !peripheral->isClaimed)
        {
            peripheral->isClaimed = true;
            return peripheral;
        }
    }

    return nullptr;
}

