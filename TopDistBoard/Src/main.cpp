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
#include "UartCobs.h"
#include <cstring>

static bool same(const uint8_t* a, const uint8_t* b, size_t n) {
  return (n == 0) || (std::memcmp(a, b, n) == 0);
}

void app_main()
{
  UART uart(PD_8, PD_9, 921600);
  UartCobs cobs(&uart);
  DigitalOut LED1(PB_0);

  // Startup blinks
  for (int i = 0; i < 5; i++) {
    LED1.write(1); HAL_Delay(200);
    LED1.write(0); HAL_Delay(200);
  }
  HAL_Delay(1000);

  const uint8_t t0_in[] = {};
  const uint8_t t0_enc[] = {0x01};

  const uint8_t t1_in[] = {0x00};
  const uint8_t t1_enc[] = {0x01, 0x01};

  const uint8_t t2_in[] = {0x11, 0x00, 0x22};
  const uint8_t t2_enc[] = {0x02, 0x11, 0x02, 0x22};

  const uint8_t t3_wrong_in[] = {0x01, 0x02, 0x03, 0x04};
  const uint8_t t3_wrong_enc[] = {0x05, 0x01, 0x02, 0x03, 0x07};

  const uint8_t t3_in[] = {0x01, 0x02, 0x03, 0x04};
  const uint8_t t3_enc[] = {0x05, 0x01, 0x02, 0x03, 0x04};

  const struct {
    const uint8_t* input;   size_t in_len;
    const uint8_t* encoded; size_t enc_len;
  } cases[] = {
    {t0_in, 0,             t0_enc, sizeof(t0_enc)},
    {t1_in, sizeof(t1_in), t1_enc, sizeof(t1_enc)},
    {t2_in, sizeof(t2_in), t2_enc, sizeof(t2_enc)},
    {t3_wrong_in, sizeof(t3_wrong_in), t3_wrong_enc, sizeof(t3_wrong_enc)},
    {t3_in, sizeof(t3_in), t3_enc, sizeof(t3_enc)},
  };

  uint8_t enc_buf[UartCobs::MAX_PACKET_SIZE + 2];
  uint8_t dec_buf[UartCobs::MAX_PACKET_SIZE];

  while (1) {
    for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
      // Encode and check against hardcoded expected
      size_t enc_len = cobs_encode(cases[i].input, cases[i].in_len, enc_buf);
      bool enc_ok = (enc_len == cases[i].enc_len) &&
                    same(enc_buf, cases[i].encoded, enc_len);

      // Decode the hardcoded encoded bytes and check we get original back
      size_t dec_len = 0;
      bool dec_ok = cobs_decode(cases[i].encoded, cases[i].enc_len,
                                dec_buf, sizeof(dec_buf), &dec_len);
      dec_ok = dec_ok && (dec_len == cases[i].in_len) &&
               same(dec_buf, cases[i].input, dec_len);

      // Also send the frame over UART so you can see it on the AD2
      cobs.write(cases[i].input, (uint16_t)cases[i].in_len);

      // Blink result
      if (enc_ok && dec_ok) {
        // 3 fast blinks = PASS
        for (int b = 0; b < 3; b++) {
          LED1.write(1); HAL_Delay(100);
          LED1.write(0); HAL_Delay(100);
        }
      } else {
        // 1 long blink = FAIL
        LED1.write(1); HAL_Delay(1000);
        LED1.write(0); HAL_Delay(200);
      }

      HAL_Delay(500);
    }

    HAL_Delay(5000);
  }
}

// void app_main()
// {
//   // log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
//   log_debug("%s","Starting app print");

//   // COBS loopback test: use PA_2 (TX) / PA_3 (RX) — physically loop these back
//   // with a jumper or AD2. Must differ from PD_8/PD_9 which are claimed by log.
//   UART uart(PD_8, PD_9, 921600); // tx, rx
//   UartCobs cobs(&uart);

//   DigitalOut LED1(PB_0);
//   while (1) {
//     // log_debug("%s","HERE2");

//     // 3–4 simple payloads
//     const uint8_t t0[] = {};                     // empty
//     const uint8_t t1[] = {0x00};                 // single zero
//     const uint8_t t2[] = {0x11,0x00,0x22};       // zero in middle // 0x02 0x11 0x02 0x22
//     const uint8_t t3[] = {0x01,0x02,0x03,0x04};  // no zeros

//     const struct { const char* name; const uint8_t* p; size_t n; } cases[] = {
//       {"empty", t0, 0},
//       {"single_zero", t1, sizeof(t1)},
//       {"mid_zero", t2, sizeof(t2)},
//       {"no_zeros", t3, sizeof(t3)},
//     };

//     uint8_t rx[UartCobs::MAX_PACKET_SIZE];

//     for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
//       // log_debug("[UARTCOBS] send %s len=%u", cases[i].name, (unsigned)cases[i].n);

//       // send out over real UART pins
//       cobs.write(cases[i].p, (uint16_t)cases[i].n);
//       // log_debug("finished write");

//       // read back from real UART pins (requires loopback/echo)
//       // size_t got = cobs.read(rx, sizeof(rx));   // WARNING: blocks forever if no echo

//       // bool equal = (got == cases[i].n) && same(rx, cases[i].p, cases[i].n);
//       // if (!equal) {
//       //     log_debug("[UARTCOBS] %s FAIL got_len=%u", cases[i].name, (unsigned)got);
//       // } else {
//       //     log_debug("[UARTCOBS] %s PASS", cases[i].name);
//       // }

//       HAL_Delay(200);
//     }

//     // log_debug("[UARTCOBS] done new change");
//     HAL_Delay(5000);
//     LED1.write(!LED1.read());
//   }
// }

// extern void uartcobs_emit_py_tests();

// void app_main()
// {

//   /* USER CODE BEGIN Init */
//   log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
//   /* USER CODE END Init */

//   log_debug("About to call emit py tests");
//   // uartcobs_emit_py_tests(); FROM JUST TESTING the encode/decode functions

//   DigitalOut LED1(PB_0);

//   while (1)
//   {
//     log_debug("%s","HERE2");
//     HAL_Delay(1000);
//     LED1.write(!LED1.read());
//   }
// }