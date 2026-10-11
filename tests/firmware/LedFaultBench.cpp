#include "startup.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32_hal.h"

// Inspect this symbol with a debugger: 0 = waiting, 1 = injection started,
// 2 = stack recursion returned, 3 = fault unexpectedly failed to trigger.
extern "C" {
volatile uint32_t led_fault_bench_stage = 0U;
}

#if defined(LED_FAULT_BENCH_STACKOVERFLOW)
#if portSTACK_GROWTH != -1
#error "The bench overflow fixture requires a downward-growing stack"
#endif

static constexpr uint32_t stack_words = 128U;
static constexpr uint32_t reserved_words = 2048U;
// The task registers only the last 128 words as its stack. A real stack
// overflow enters the reserved prefix, keeping other tasks/TCBs out of reach.
static StackType_t stack_storage[reserved_words + stack_words];
static StaticTask_t task_storage;

__attribute__((noinline)) static void consume_stack(unsigned depth)
{
    volatile uint32_t scratch[32];
    for (unsigned i = 0; i < 32U; ++i) { scratch[i] = depth + i; }
    if (depth != 0U) { consume_stack(depth - 1U); }
    // Use the frame after the recursive call to prevent tail-call elimination.
    __asm volatile ("" : : "m"(scratch) : "memory");
}

static void overflow_task(void *)
{
    led_fault_bench_stage = 1U;
    taskENTER_CRITICAL();
    consume_stack(8U); // > 512-byte registered stack, < 8192-byte reserved prefix
    led_fault_bench_stage = 2U;
    taskEXIT_CRITICAL();
    taskYIELD(); // FreeRTOS checks the overwritten bottom-of-stack sentinels
    led_fault_bench_stage = 3U;
    for (;;) { taskYIELD(); }
}
#endif

void app_main()
{
    vTaskDelay(pdMS_TO_TICKS(1000U));
#if defined(LED_FAULT_BENCH_HARDFAULT)
    led_fault_bench_stage = 1U;
    // An undefined instruction escalates to HardFault when UsageFault is off.
    SCB->SHCSR &= ~SCB_SHCSR_USGFAULTENA_Msk;
    __DSB();
    __ISB();
    __asm volatile ("udf #0");
    led_fault_bench_stage = 3U;
#elif defined(LED_FAULT_BENCH_STACKOVERFLOW)
    TaskHandle_t task = xTaskCreateStatic(overflow_task, "fault-bench", stack_words,
        nullptr, 2U, stack_storage + reserved_words, &task_storage);
    configASSERT(task != nullptr);
#else
#error "Choose HARDFAULT or STACKOVERFLOW for the fault bench firmware"
#endif
    for (;;) { vTaskDelay(portMAX_DELAY); }
}
