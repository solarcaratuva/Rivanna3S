
#include "UART.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"
#include "log.h"


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

    // Register this UART instance
    for (size_t i = 0; i < UART_PERIPHERAL_COUNT; ++i) {
        if (UART_Peripherals[i].handle == huart->Instance) {
            uart_instance_map[i] = this;
            break;
        }
    }

    initialized = true; 
}



int UART::read(uint8_t *buffer, uint16_t length){
	if(!initialized) {
    log_warn("UART read failed: UART not initialized");
        return -1;
    }

    last_error = HAL_UART_ERROR_NONE;

    rxTask = Thread::get_task_handle();

    UART_Start_Receive_IT(huart, buffer, length);

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    return last_error;
}

int UART::read(uint8_t *buffer, uint16_t length, uint32_t timeout_ms){
	if(!initialized) {
    log_warn("UART read failed: UART not initialized");
        return -1;
	}

    last_error = HAL_UART_ERROR_NONE;

    rxTask = Thread::get_task_handle();

    HAL_UART_Receive_IT(huart, buffer, length);

    TickType_t timeoutTicks =
    (timeout_ms == portMAX_DELAY)
        ? portMAX_DELAY
        : pdMS_TO_TICKS(timeout_ms);

    uint32_t notified = ulTaskNotifyTake(pdTRUE, timeoutTicks);

    // -------- CASE 1: TIMEOUT --------
    if (notified == 0) {
        // Abort transfer safely
        HAL_UART_Abort_IT(huart);
        rxTask = nullptr;
        return -2;  // timeout error
    }

    return last_error; 
}

int UART::write(uint8_t* buffer, uint16_t length) {
	if(!initialized) {
    log_warn("UART write failed: UART not initialized");
        return -1;
	}

    last_error = HAL_UART_ERROR_NONE;
    txTask = Thread::get_task_handle();

    HAL_UART_Transmit_IT(huart, buffer, length);

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    return last_error;
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
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(
            uart->txTask,
            0,
            eNoAction,
            &xHigherPriorityTaskWoken
        );

        uart->txTask = nullptr;
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return;
    }
}

extern "C"{
    void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
        UART* uart = UART::find_from_handle(huart);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(
            uart->rxTask,
            0,
            eNoAction,
            &xHigherPriorityTaskWoken
        );

        uart->rxTask = nullptr;
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
            xTaskNotifyFromISR(
                uart->rxTask,
                0,
                eNoAction,
                &woken
            );
            uart->rxTask = nullptr;
        }

        // Wake TX task if waiting
        if (uart->txTask) {
            xTaskNotifyFromISR(
                uart->txTask,
                0,
                eNoAction,
                &woken
            );
            uart->txTask = nullptr;
        }

        portYIELD_FROM_ISR(woken);
    }
}



