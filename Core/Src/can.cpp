#include "can.h"
#include <string.h> // memcpy
#include "pinmap.h"

// -----------------------------------------------------------------------------
// Helper: convert length <-> DLC
// -----------------------------------------------------------------------------

CAN::CAN(Pin tx, Pin rx, uint32_t baudrate)
    m_txMutex()
{
    fdcan_periph = findCANPin(tx, rx);
    if (fdcan_periph == nullptr)
    {
        initialized == false;
        return;
    }

    if (fdcan_periph)
    {
        in
    }
    FDCAN_GlobalTypeDef *fdcan_handle = fdcan_periph->handle;
    FDCAN_init(fdcan_periph->handle);
    uint8_t tx_af = get_FDCAN_AF(fdcan_periph->handle, &tx, TX);
    uint8_t rx_af = get_FDCAN_AF(fdcan_periph->handle, &rx, RX);
    HAL_FDCAN_MspInit_custom(fdcan_periph->handle, tx, tx_af);
    HAL_FDCAN_MspInit_custom(fdcan_periph->handle, rx, rx_af);
    HAL_FDCAN_Start(fdcan_periph->handle);

    // Default Tx header setup; fields that change per-frame will be set in write().
    m_txHeader.IdType = FDCAN_STANDARD_ID;
    m_txHeader.TxFrameType = FDCAN_DATA_FRAME;
    m_txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    m_txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    m_txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    m_txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    m_txHeader.MessageMarker = 0;

    return fdcan_handle;
}

// Write a raw frame
int CAN::write(const SerializedCanMessage &msg)
{
    m_txMutex.lock();
    m_txHeader.Identifier = msg.id;
    m_txHeader.DataLength = bytesToDlc(msg.len);

    // HAL expects a uint8_t* to data
    HAL_StatusTypeDef status =
        HAL_FDCAN_AddMessageToTxFifoQ(m_hfdcan, &m_txHeader,
                                      const_cast<uint8_t *>(msg.data));

    m_txMutex.unlock();

    if (status != HAL_OK)
    {
        return -4;
    }

    return 0;
}

// Write a logical CanMessage
int CAN::write(CanMessage *msg)
{
    SerializedCanMessage scm;
    msg->serialize(&scm);

    return write(scm);
}

// Poll RX FIFO0
int CAN::read(SerializedCanMessage *msg)
{
    // Check how many frames are pending in FIFO0
    uint32_t pending = HAL_FDCAN_GetRxFifoFillLevel(m_hfdcan, FDCAN_RX_FIFO0);
    if (pending == 0)
    {
        // No message available right now
        return 0;
    }
    uint8_t rxData[8] = {0};

    HAL_StatusTypeDef status =
        HAL_FDCAN_GetRxMessage(m_hfdcan, FDCAN_RX_FIFO0, &m_rxHeader, rxData);

    if (status != HAL_OK)
    {
        Error_Handler();
    }

    msg->id = static_cast<uint16_t>(m_rxHeader.Identifier);
    msg->len = dlcToBytes(m_rxHeader.DataLength);
    if (msg->len > 8)
    {
        msg->len = 8;
    }
    memcpy(msg->data, rxData, msg->len);

    return 1;
}

uint32_t CAN::bytesToDlc(uint8_t len) const
{
    // Clamp to 8 bytes; extend if you move to CAN FD later.
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
        return 8; // conservative fallback
    }
}

// -----------------------------------------------------------------------------
// CAN implementation
// -----------------------------------------------------------------------------

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
