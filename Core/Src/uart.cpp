
#include "stm32h7xx_hal.h"
#include "pinmap.h"

extern UART_HandleTypeDef huart4;

class uart{
    public:
        void read(uint8_t *buffer, size_t length){
            HAL_UART_Receive(&huart4, buffer, length, HAL_MAX_DELAY);
        }
        void write(uint8_t* buffer, size_t length) {
            HAL_UART_Transmit(&huart4, buffer, length, HAL_MAX_DELAY);
        }
        uart(Pin pin1,  Pin pin2, uint32_t baud){
            
        }
        
        
}
