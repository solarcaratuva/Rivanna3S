#include "can.h"
#include <string.h> // memcpy
#include "pinmap.h"
#include "log.h"
#include "peripheralmap.h"
#include "lock.h"
#include "fdcan.h"

// extern "C" void HAL_FDCAN_MspInit_custom(FDCAN_GlobalTypeDef* fdcanHandle, Pin pin, uint8_t af);
// extern "C" FDCAN_HandleTypeDef* FDCAN_init(FDCAN_GlobalTypeDef* fdcan, uint32_t baudrate);


CAN::CAN(Pin tx, Pin rx, uint32_t baudrate)
   
{
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
    HAL_FDCAN_Start(hfdcan);

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


int CAN::write(const SerializedCanMessage &msg)
{
    if (!initialized) {
        log_warn("CAN write failed: CAN not initialized");
        return 1;
    }

    instance_lock.lock();
    txHeader.Identifier = msg.id;
    txHeader.DataLength = bytesToDlc(msg.len);

    // HAL expects a uint8_t* to data
    HAL_StatusTypeDef status =
        HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader,
                                      const_cast<uint8_t *>(msg.data));

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

    return write(scm);
}


int CAN::read(SerializedCanMessage *msg)
{
    if (!initialized) {
        log_warn("CAN read failed: CAN not initialized");
        return 1;
    }

    instance_lock.lock();

    // Check how many frames are pending in FIFO0
    uint32_t pending = 0;
    while (pending == 0) {
        pending = HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0);
    }

    // Check if the RX queue is full (field name differs across STM32 families).
#if defined(STM32G474xx)
    if (pending > 0) {
        // G4 HAL does not expose FIFO element count in the init struct.
        log_warn("CAN RX pending=%lu; monitor for dropped frames", pending);
    }
#else
    if (pending == hfdcan->Init.RxFifo0ElmtsNbr) {
        log_warn("CAN RX FIFO0 full; messages were likely dropped");
    }
#endif
    
    uint8_t rxData[8] = {0};

    HAL_StatusTypeDef status =
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData);

    if (status != HAL_OK)
    {
        log_warn("CAN read failed: status %d, error code %lx", status, hfdcan->ErrorCode);
        instance_lock.unlock();
        return 2;
    }

    msg->id = static_cast<uint16_t>(rxHeader.Identifier);
    msg->len = dlcToBytes(rxHeader.DataLength);
    memcpy(msg->data, rxData, msg->len);

    instance_lock.unlock();

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
