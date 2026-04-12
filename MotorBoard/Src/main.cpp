/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "UART.h"
#include "AnalogIn.h"
#include "Timeout.h"
#include "Clock.h"
#include "thread.h"
#include "Timeout.h"
#include "lock.h"
#include "log.h"
#include "can.h"
#include "BPSCanStructs.h"
#include "Rivanna3CanStructs.h"
#include "FiniteQueue.h"

#define BLOCK_SIZE      64 // Must match the Python sender's block size
#define MAX_FIRMWARE_SIZE (4096 * 1024)  // 4MB, adjust to your flash size
#define CRC16_INIT      0xFFFF

Clock clock = Clock();
FiniteQueue transmitqueue(100,BLOCK_SIZE);
Thread codeReaderThread;

// --- CRC16 HQXA (matches Python's binascii.crc_hqx) ---
static uint16_t crc16_hqx(const uint8_t* data, uint16_t len) {
    uint16_t crc = CRC16_INIT;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

// --- Read exactly `len` bytes with timeout ---
static bool uart_read_exact(UART& uart, uint8_t* buf, uint16_t len, uint32_t timeout_ms) {
    return uart.read(buf, len, timeout_ms) == 0;
}

// --- Read a line (up to '\n') into buf, returns length ---
static int uart_read_line(UART& uart, char* buf, uint16_t max_len, uint32_t timeout_ms) {
    for (uint16_t i = 0; i < max_len - 1; i++) {
        if (uart.read((uint8_t*)&buf[i], 1, timeout_ms) != 0)
            return -1;  // timeout
        if (buf[i] == '\n') {
            buf[i + 1] = '\0';
            return i + 1;
        }
    }
    return -1;
}

// --- Main upload handler ---
static bool handle_upload(UART& uart) {
    uint8_t  block[BLOCK_SIZE];
    uint8_t  packet[BLOCK_SIZE + 2];  // block + 2-byte CRC
    uint32_t offset = 0;

    

    while (offset < MAX_FIRMWARE_SIZE) {
        
        uart.write((uint8_t*)"REQ", 3);

        // Receive block + CRC
        if (!uart_read_exact(uart, packet, sizeof(packet), 1000)) {
            //timeout - assume done
            uart.write((uint8_t*)"DONE", 4);
            return true;
        }

        // Verify CRC
        uint16_t received_crc = ((uint16_t)packet[BLOCK_SIZE] << 8) | packet[BLOCK_SIZE + 1];
        uint16_t computed_crc = crc16_hqx(packet, BLOCK_SIZE);

        if (computed_crc != received_crc) {
            uart.write((uint8_t*)"NACK", 4);
            continue;
        }

        // Store block
        if (offset + BLOCK_SIZE > MAX_FIRMWARE_SIZE) {
            uart.write((uint8_t*)"NACK", 4);
            return false;  // overflow
        }

        int result = transmitqueue.append_to_back(packet, portMAX_DELAY);
        if (result != 0) {
            if(result == -2){
                uart.write((uint8_t*)"FAIL", 4);
            }
            else if(result == -1){ 
                uart.write((uint8_t*)"INIT\n", 4);
            }
            uart.write((uint8_t*)"NACK", 4);
            continue;  // queue full
        }
        offset += BLOCK_SIZE;

        uart.write((uint8_t*)"ACK\n", 4);  //Python script reads 4 bytes

        // // Check if this was the last block (PC sends no more REQs after final ACK)
        // // Peek: try to read another packet with a short timeout; if nothing comes, we're done
        // uint8_t probe;
        // if (uart.read(&probe, 1, 100) == -2) {
        //     // Timeout — PC has stopped sending, upload complete
        //     firmware_size = offset;
        //     uart.write((uint8_t*)"DONE", 4);
        //     return true;
        // }
        
    }

    uart.write((uint8_t*)"ERR\n", 4);
    return false;
}

void code_reader() {
    // This code will periodically pop from the finit queue
    while (1) {
        uint8_t block[BLOCK_SIZE];
        if (transmitqueue.get(block, BLOCK_SIZE)) {
            // Process the block (e.g., send over CAN, write to flash, etc.)
            //log_info("Read block with first byte: %02X", block[0]);
        }
        clock.sleep_for(2);  // Adjust delay as needed
    }
}

void app_main()
{
    //Start the code reader thread
    codeReaderThread.start(code_reader);

    //Allocate firmware buffer
    UART USB_UART(PC_12, PD_2, 115200);

    char cmd_buf[32];

    while (1) {
        // Wait for UPLOAD command: "UPLOAD <board_id>\n"
        if (uart_read_line(USB_UART, cmd_buf, sizeof(cmd_buf), portMAX_DELAY) < 0)
            continue;

        int board_id = -1;
        board_id = cmd_buf[0];
        //convert ASCII digit to integer
        if(board_id >= '0' && board_id <= '9'){
            board_id -= '0';
        }
        if (board_id < 0 || board_id > 4) {
            continue;  // not a valid command, keep waiting
        }

        // board_id could be used to route to a sub-board via CAN, etc.
        // For now, just receive and store the firmware.
        handle_upload(USB_UART);
    }

}


