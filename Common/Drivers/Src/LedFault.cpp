#include "LedFault.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pindef.h"

namespace {
struct FaultLed {
    GPIO_TypeDef *port;
    uint16_t pin;
    bool active_high;
};

// Board-specific constants live in flash.
const FaultLed fault_leds[] = {
    {FAULT_LED_1_PORT, FAULT_LED_1_PIN, FAULT_LED_1_ACTIVE_HIGH},
    {FAULT_LED_2_PORT, FAULT_LED_2_PIN, FAULT_LED_2_ACTIVE_HIGH},
};
constexpr unsigned fault_led_count = sizeof(fault_leds) / sizeof(fault_leds[0]);
static_assert(fault_led_count == 2U, "Every board must configure two fault LEDs");

volatile bool fault_tick_active = false;
uint32_t fault_tick_ms;

void write_led(const FaultLed &led, bool on)
{
    led.port->BSRR = (on == led.active_high)
        ? led.pin : static_cast<uint32_t>(led.pin) << 16U;
}

void enable_gpio_clock(GPIO_TypeDef *port)
{
    // These HAL macros expand to RCC register writes and a clock readback.
    if (port == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); }
    else if (port == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); }
    else if (port == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); }
#if defined(GPIOE)
    else if (port == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); }
#endif
}

void configure_led(const FaultLed &led)
{
    enable_gpio_clock(led.port);
    write_led(led, true); // preload ON before switching to output mode
    led.port->OTYPER &= ~static_cast<uint32_t>(led.pin);
    for (uint32_t index = 0; index < 16U; ++index) {
        if ((led.pin & (1U << index)) == 0U) { continue; }
        const uint32_t shift = index * 2U;
        led.port->OSPEEDR &= ~(3UL << shift);
        led.port->PUPDR &= ~(3UL << shift);
        led.port->MODER = (led.port->MODER & ~(3UL << shift)) | (1UL << shift);
    }
}

void start_fault_tick(void)
{
    // The terminal handler owns SysTick from now on. Poll its wrap flag instead
    // of enabling an interrupt that cannot preempt HardFault or a critical hook.
    SysTick->CTRL = 0U;
    fault_tick_ms = 0U;
    const uint32_t cycles_per_ms = SystemCoreClock / 1000U;
    SysTick->VAL = 0U;
    fault_tick_active = true;
    uwTickFreq = HAL_TICK_FREQ_1KHZ;
    if (cycles_per_ms != 0U && cycles_per_ms - 1U <= SysTick_LOAD_RELOAD_Msk) {
        SysTick->LOAD = cycles_per_ms - 1U;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    }
    __DSB();
    __ISB();
}

void blink_pulses(const FaultLed *leds, unsigned count, uint8_t pulses, uint32_t delay_ms)
{
    for (uint8_t pulse = 0; pulse < pulses; ++pulse) {
        for (unsigned i = 0; i < count; ++i) { write_led(leds[i], true); }
        HAL_Delay(delay_ms);
        for (unsigned i = 0; i < count; ++i) { write_led(leds[i], false); }
        HAL_Delay(delay_ms);
    }
}

void blink_pattern(const FaultLed *leds, unsigned count, uint8_t pattern)
{
    blink_pulses(leds, count, pattern & 0x0FU, LED_FAULT_SLOW_BLINK_MS);
    blink_pulses(leds, count, (pattern >> 4U) & 0x0FU, LED_FAULT_FAST_BLINK_MS);
    HAL_Delay(LED_FAULT_PATTERN_PAUSE_MS);
}
} // namespace

// Keep normal HAL tick behavior exactly as supplied by STM32. Only a terminal
// fault uses the polled timer. HAL_Delay itself remains the vendor implementation.
extern "C" uint32_t HAL_GetTick(void)
{
    if (!fault_tick_active) { return uwTick; }
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U) { ++fault_tick_ms; }
    return fault_tick_ms;
}

void LedFault_fault_signal_blink(GPIO_TypeDef *port, uint16_t pin, uint8_t pattern)
{
    const FaultLed led = {port, pin, true};
    blink_pattern(&led, 1U, pattern);
}

void LedFault_halt(enum LedFault_Faults fault)
{
    __disable_irq();
    for (const FaultLed &led : fault_leds) { configure_led(led); }
    __DSB(); // make both ON before timer setup or the first HAL_Delay
    start_fault_tick();
    while (true) {
        blink_pattern(fault_leds, fault_led_count, static_cast<uint8_t>(fault));
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    LedFault_halt(LedFault_MallocFailed);
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    LedFault_halt(LedFault_StackOverflow);
}
