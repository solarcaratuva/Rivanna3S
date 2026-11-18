#include "UART.h"
#include "stm32_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"
#include "log.h"
#include "semphr.h"


extern "C" void HAL_UART_MspInit_custom(USART_TypeDef* uartHandle, Pin pin, uint8_t af);
extern "C" UART_HandleTypeDef* UART_init(USART_TypeDef* uart, uint32_t baudrate);

UART* uart_instance_map[12] = { nullptr };

// Constructor
UART::UART(Pin tx, Pin rx, uint32_t baud)
    : tx_(tx), rx_(rx), baud_(baud) {
	uart_periph = find_uart_pins(tx, rx);
    if(uart_periph == nullptr) {
        initialized = false;
        log_warn("UART init failed: no matching peripheral for TX/RX pins");
        return;
    }
    uart_periph->rxd_used = rx;
    uart_periph->txd_used = tx;
    
    gpio_clock_enable(tx.block);
    gpio_clock_enable(rx.block);

    uint8_t tx_af = get_UART_AF(uart_periph->handle, &tx, TX);
    uint8_t rx_af = get_UART_AF(uart_periph->handle, &rx, RX);
    HAL_UART_MspInit_custom(uart_periph->handle, tx, tx_af);
    HAL_UART_MspInit_custom(uart_periph->handle, rx, rx_af);
    
    huart = UART_init(uart_periph->handle, baud);

    // Clear any startup glitches/errors once
    __HAL_UART_CLEAR_IT(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_FEF | UART_CLEAR_PEF);

    // Register this UART instance
    for (size_t i = 0; i < UART_PERIPHERAL_COUNT; ++i) {
        if (UART_Peripherals[i].handle == huart->Instance) {
            uart_instance_map[i] = this;
            break;
        }
    }

    // Waiting for incoming bytes must not prevent outgoing log writes.
    rx_mutex = xSemaphoreCreateMutex();
    tx_mutex = xSemaphoreCreateMutex();

    initialized = rx_mutex && tx_mutex;
}



int UART::read(uint8_t *buffer, uint16_t length){
    return read(buffer, length, portMAX_DELAY);
}

int UART::read(uint8_t *buffer, uint16_t length, uint32_t timeout_ms){
    return read_impl(buffer, length, timeout_ms, nullptr);
}

int UART::read_until_idle(uint8_t *buffer, uint16_t length, uint16_t &received,
                          uint32_t timeout_ms) {
    received = 0;
    return read_impl(buffer, length, timeout_ms, &received);
}

int UART::read_impl(uint8_t *buffer, uint16_t length, uint32_t timeout_ms,
                    uint16_t *received) {
	if(initialized) {
        if (xSemaphoreTake(rx_mutex, portMAX_DELAY) != pdTRUE) return -3;
        rx_error = HAL_UART_ERROR_NONE;
        rx_received = 0;

        rxTask = Thread::get_task_handle();

        const HAL_StatusTypeDef status = received
            ? HAL_UARTEx_ReceiveToIdle_IT(huart, buffer, length)
            : HAL_UART_Receive_IT(huart, buffer, length);
        if (status != HAL_OK) {
            rxTask = nullptr;
            xSemaphoreGive(rx_mutex);
            return -1;
        }

        TickType_t timeoutTicks = (timeout_ms == portMAX_DELAY)
                                      ? portMAX_DELAY
                                      : pdMS_TO_TICKS(timeout_ms);

        uint32_t notified = ulTaskNotifyTake(pdTRUE, timeoutTicks);

        // -------- CASE 1: TIMEOUT --------
        if (notified == 0) {
            // Abort transfer safely
            rxTask = nullptr;
            HAL_UART_AbortReceive(huart);
            ulTaskNotifyTake(pdTRUE, 0);
            xSemaphoreGive(rx_mutex);
            return -2;  // timeout error
        }

        rxTask = nullptr;
        const int error = rx_error;
        if (error != HAL_UART_ERROR_NONE) HAL_UART_AbortReceive(huart);
        if (received && error == HAL_UART_ERROR_NONE) *received = rx_received;
        xSemaphoreGive(rx_mutex);
        return error;
	}

    return -1; //unitialized
}

int UART::write(uint8_t* buffer, uint16_t length, uint32_t timeout_ms) {
	if(initialized) {
        if (xSemaphoreTake(tx_mutex, portMAX_DELAY) != pdTRUE) return -3;
        tx_error = HAL_UART_ERROR_NONE;
        txTask = Thread::get_task_handle();

		if (HAL_UART_Transmit_IT(huart, buffer, length) != HAL_OK) {
            txTask = nullptr;
            xSemaphoreGive(tx_mutex);
            return -1;
        }

        TickType_t timeoutTicks = (timeout_ms == portMAX_DELAY)
                                      ? portMAX_DELAY
                                      : pdMS_TO_TICKS(timeout_ms);

        uint32_t notified = ulTaskNotifyTake(pdTRUE, timeoutTicks);

        if (notified == 0) {
            txTask = nullptr;
            HAL_UART_AbortTransmit(huart);
            // Discard a late completion so it cannot satisfy the next write's wait.
            // NOTE: this clears the task's whole default notification value, so a
            // task must not use notification index 0 for anything but this driver.
            ulTaskNotifyTake(pdTRUE, 0);
            xSemaphoreGive(tx_mutex);
            return -2;
        }

        txTask = nullptr;
        if (tx_error != HAL_UART_ERROR_NONE) HAL_UART_AbortTransmit(huart);
        xSemaphoreGive(tx_mutex);

        return tx_error;
	}

    return -1; //unitialized
}
        
UART_Peripheral* UART::find_uart_pins(Pin tx, Pin rx) {
    for (size_t i = 0; i < UART_PERIPHERAL_COUNT; ++i) {
        UART_Peripheral* peripheral = &UART_Peripherals[i];
        if ((peripheral->txd_valid_pins & tx.universal_mask) &&
            (peripheral->rxd_valid_pins & rx.universal_mask)) {
            if (!peripheral->isClaimed) {
                peripheral->isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching peripheral found
}

UART* UART::find_from_handle(UART_HandleTypeDef* huart){
    if (!huart) return nullptr;

    for (size_t i = 0; i < UART_PERIPHERAL_COUNT; ++i) {
        if (UART_Peripherals[i].handle == huart->Instance) {
            return uart_instance_map[i];
        }
    }
    return nullptr;
}


// Call Back functions
extern "C"{

    void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
        UART* uart = UART::find_from_handle(huart);
        if (!uart) return;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (uart->txTask) {
            xTaskNotifyFromISR(
                uart->txTask,
                0,
                eIncrement,
                &xHigherPriorityTaskWoken
            );
            uart->txTask = nullptr;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return;
    }
}

extern "C"{
    void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
        UART* uart = UART::find_from_handle(huart);
        if (!uart || !uart->rxTask) return;
        uart->rx_received = size;
        BaseType_t woken = pdFALSE;
        xTaskNotifyFromISR(uart->rxTask, 0, eIncrement, &woken);
        uart->rxTask = nullptr;
        portYIELD_FROM_ISR(woken);
    }

    void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
        UART* uart = UART::find_from_handle(huart);
        if (!uart) return;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (uart->rxTask) {
            xTaskNotifyFromISR(
                uart->rxTask,
                0,
                eIncrement,
                &xHigherPriorityTaskWoken
            );
            uart->rxTask = nullptr;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        return;
    }
}

extern "C"{
    void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
        UART* uart = UART::find_from_handle(huart);
        if (!uart) return;

        // Capture HAL error flags
        uart->last_error = HAL_UART_GetError(huart);

        BaseType_t woken = pdFALSE;

        // Wake RX task if waiting
        if (uart->rxTask) {
            uart->rx_error = uart->last_error;
            xTaskNotifyFromISR(
                uart->rxTask,
                0,
                eIncrement,
                &woken
            );
            uart->rxTask = nullptr;
        }

        // Wake TX task if waiting
        if (uart->txTask) {
            uart->tx_error = uart->last_error;
            xTaskNotifyFromISR(
                uart->txTask,
                0,
                eIncrement,
                &woken
            );
            uart->txTask = nullptr;
        }

        portYIELD_FROM_ISR(woken);
    }
}
