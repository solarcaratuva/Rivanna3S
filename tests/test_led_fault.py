"""Run real fault signaling and vendor HAL_Delay against simulated registers."""

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def extract_function(source, signature):
    text = source.read_text()
    start = text.index(signature)
    body = text.index("{", start)
    depth = 1
    end = body + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


HAL_SHIM = r"""
#ifndef STM32_HAL_H
#define STM32_HAL_H
#include <stdint.h>
#ifdef __cplusplus
struct OutputRegister {
    uint32_t state = 0xA5A5U;
    void operator=(uint32_t bsrr) {
        state = (state | (bsrr & 0xFFFFU)) & ~(bsrr >> 16U);
    }
};
struct GPIO_TypeDef {
    uint32_t MODER = 0xFFFFFFFFU, OTYPER = 0xFFFFU;
    uint32_t OSPEEDR = 0xFFFFFFFFU, PUPDR = 0xFFFFFFFFU;
    OutputRegister BSRR;
};
struct ControlRegister {
    uint32_t value = 0U;
    unsigned reads = 0U;
    void operator=(uint32_t v) { value = v; }
    operator uint32_t();
};
struct SysTick_Type { ControlRegister CTRL; uint32_t LOAD = 0U, VAL = 0U; };
extern GPIO_TypeDef gpio_a, gpio_b, gpio_c, gpio_e;
extern SysTick_Type systick;
#define GPIOA (&gpio_a)
#define GPIOB (&gpio_b)
#define GPIOC (&gpio_c)
#define GPIOE (&gpio_e)
#define SysTick (&systick)
#define __HAL_RCC_GPIOA_CLK_ENABLE() test_gpio_clock(1U)
#define __HAL_RCC_GPIOB_CLK_ENABLE() test_gpio_clock(2U)
#define __HAL_RCC_GPIOC_CLK_ENABLE() test_gpio_clock(4U)
#define __HAL_RCC_GPIOE_CLK_ENABLE() test_gpio_clock(8U)
extern "C" {
#else
typedef struct GPIO_TypeDef GPIO_TypeDef;
#endif
#define GPIO_PIN_0 ((uint16_t)0x0001U)
#define GPIO_PIN_1 ((uint16_t)0x0002U)
#define GPIO_PIN_5 ((uint16_t)0x0020U)
#define GPIO_PIN_9 ((uint16_t)0x0200U)
#define GPIO_PIN_13 ((uint16_t)0x2000U)
#define SysTick_CTRL_ENABLE_Msk 1U
#define SysTick_CTRL_TICKINT_Msk 2U
#define SysTick_CTRL_CLKSOURCE_Msk 4U
#define SysTick_CTRL_COUNTFLAG_Msk 0x10000U
#define SysTick_LOAD_RELOAD_Msk 0xFFFFFFU
#define HAL_TICK_FREQ_1KHZ 1U
#define HAL_MAX_DELAY 0xFFFFFFFFU
extern volatile uint32_t uwTick;
extern uint32_t uwTickFreq, SystemCoreClock;
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t ms);
void test_disable_irq(void);
void test_gpio_clock(uint32_t bit);
#define __disable_irq() test_disable_irq()
#define __DSB() ((void)0)
#define __ISB() ((void)0)
#ifdef __cplusplus
}
#endif
#endif
"""

PINDEF_SHIM = r"""
#define FAULT_LED_1_PORT GPIOC
#define FAULT_LED_1_PIN GPIO_PIN_1
#define FAULT_LED_1_ACTIVE_HIGH 1
// Different port and polarity exercise board-specific configuration.
#define FAULT_LED_2_PORT GPIOB
#define FAULT_LED_2_PIN GPIO_PIN_9
#define FAULT_LED_2_ACTIVE_HIGH 0
"""

HARNESS = r"""
#include <cassert>
#include <csetjmp>
#include <cstdio>
#include <cstring>
#include "LedFault.h"
#include "FreeRTOSConfig.h"
#include "task.h"

GPIO_TypeDef gpio_a, gpio_b, gpio_c, gpio_e;
SysTick_Type systick;
volatile uint32_t uwTick = 12345U;
uint32_t uwTickFreq = 10U, SystemCoreClock = 16000000U;
static bool irq_disabled, timer_stopped;
static unsigned gpio_clocks, delay_count;
static uint32_t delays[40];
static bool primary_states[40], secondary_states[40];
static jmp_buf escape;

extern "C" {
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t, char *);
void __real_HAL_Delay(uint32_t);
int c_assert_uses_once(void);
}

bool primary_on() { return (gpio_c.BSRR.state & 2U) != 0U; }
bool secondary_on()
{
    const bool high = (EXPECTED_SECOND_PORT->BSRR.state & EXPECTED_SECOND_PIN) != 0U;
    return high == (EXPECTED_SECOND_ACTIVE_HIGH != 0);
}
bool all_on() { return primary_on() && secondary_on(); }
void test_disable_irq() { irq_disabled = true; }
void test_gpio_clock(uint32_t bit) { gpio_clocks |= bit; }

ControlRegister::operator uint32_t()
{
    ++reads;
    if (timer_stopped || (value & SysTick_CTRL_ENABLE_Msk) == 0U) {
        if (reads > 1000U) {
            // Escape a genuinely stalled vendor HAL_Delay; both LEDs must
            // already be ON at its first call.
            assert(delay_count == 1U && all_on());
            longjmp(escape, 2);
        }
        return value;
    }
    assert((value & SysTick_CTRL_TICKINT_Msk) == 0U);
    return value | (reads % 3U == 0U ? SysTick_CTRL_COUNTFLAG_Msk : 0U);
}

static void check_configuration()
{
    assert(irq_disabled);
    assert(gpio_c.MODER == ((0xFFFFFFFFU & ~(3U << 2U)) | (1U << 2U)));
    assert(gpio_c.OTYPER == (0xFFFFU & ~2U));
    assert(gpio_c.OSPEEDR == (0xFFFFFFFFU & ~(3U << 2U)));
    assert(gpio_c.PUPDR == gpio_c.OSPEEDR);
    GPIO_TypeDef &other = *EXPECTED_SECOND_PORT;
    const unsigned other_clock = &other == GPIOA ? 1U : &other == GPIOB ? 2U : 8U;
    assert(gpio_clocks == (4U | other_clock));
    const uint32_t mask = EXPECTED_SECOND_PIN;
    const uint32_t shift = __builtin_ctz(mask) * 2U;
    assert(other.MODER == ((0xFFFFFFFFU & ~(3U << shift)) | (1U << shift)));
    assert(other.OTYPER == (0xFFFFU & ~mask));
    assert(other.OSPEEDR == (0xFFFFFFFFU & ~(3U << shift)));
    assert(other.PUPDR == other.OSPEEDR);
    GPIO_TypeDef *const ports[] = {GPIOA, GPIOB, GPIOE};
    for (GPIO_TypeDef *unused : ports) {
        if (unused == &other) continue;
        assert(unused->MODER == 0xFFFFFFFFU && unused->OTYPER == 0xFFFFU);
        assert(unused->OSPEEDR == 0xFFFFFFFFU && unused->PUPDR == 0xFFFFFFFFU);
        assert(unused->BSRR.state == 0xA5A5U);
    }
    assert(uwTick == 12345U); // interrupt-driven tick never advances in faults
}

extern "C" void __wrap_HAL_Delay(uint32_t ms)
{
    assert(delay_count < 40U);
    check_configuration();
    if (delay_count == 0U) {
        assert(all_on());
        if (SystemCoreClock != 0U) assert(systick.LOAD == 15999U);
    }
    delays[delay_count] = ms;
    primary_states[delay_count] = primary_on();
    secondary_states[delay_count++] = secondary_on();
    uint32_t before = HAL_GetTick();
    __real_HAL_Delay(ms); // exact function extracted from the selected STM32 HAL
    uint32_t elapsed = HAL_GetTick() - before;
    assert(elapsed >= ms && elapsed <= ms + 3U);
    if (ms == 1500U) longjmp(escape, 1);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    // Before a fault, HAL_GetTick preserves normal HAL behavior and leaves
    // SysTick alone. The assertion macro must also work from C callers.
    assert(HAL_GetTick() == 12345U && systick.CTRL.reads == 0U);
    assert(c_assert_uses_once() == 1);
    if (strcmp(argv[1], "normal") == 0) return 0;

    enum LedFault_Faults fault = LedFault_HardFault;
    unsigned slow = 1U, fast = 0U;
    if (strcmp(argv[1], "stack") == 0) { fault = LedFault_StackOverflow; slow = 2; }
    if (strcmp(argv[1], "malloc") == 0) { fault = LedFault_MallocFailed; slow = 3; }
    if (strcmp(argv[1], "mem") == 0) { fault = LedFault_MemManage; slow = 4; }
    if (strcmp(argv[1], "bus") == 0) { fault = LedFault_BusFault; slow = 5; }
    if (strcmp(argv[1], "usage") == 0) { fault = LedFault_UsageFault; slow = 6; }
    if (strcmp(argv[1], "nmi") == 0) { fault = LedFault_NMI; slow = 7; }
    if (strcmp(argv[1], "secure") == 0) { fault = LedFault_SecureFault; slow = 8; }
    if (strcmp(argv[1], "assert") == 0) { fault = LedFault_RTOSAssert; fast = 1; }
    if (strcmp(argv[1], "stopped") == 0) timer_stopped = true;
    if (strcmp(argv[1], "clockzero") == 0) SystemCoreClock = 0U;

    int outcome = setjmp(escape);
    if (outcome == 0) {
        if (fault == LedFault_StackOverflow)
            vApplicationStackOverflowHook((TaskHandle_t)1, (char *)1);
        if (fault == LedFault_MallocFailed) vApplicationMallocFailedHook();
        if (fault == LedFault_RTOSAssert) configASSERT(0);
        LedFault_halt(fault);
    }
    if (timer_stopped || SystemCoreClock == 0U) {
        assert(outcome == 2 && all_on());
        return 0;
    }
    assert(outcome == 1 && delay_count == 2U * (slow + fast) + 1U);
    for (unsigned pulse = 0; pulse < slow + fast; ++pulse) {
        unsigned i = 2U * pulse;
        uint32_t duration = pulse < slow ? 500U : 200U;
        assert(delays[i] == duration && delays[i + 1] == duration);
        assert(primary_states[i] && !primary_states[i + 1]);
        assert(secondary_states[i] && !secondary_states[i + 1]);
    }
    assert(delays[delay_count - 1] == 1500U);
    assert(!primary_on() && !secondary_on());
    return 0;
}
"""


class LedFaultTest(unittest.TestCase):
    def test_real_hal_delay_all_families(self):
        cc, cxx = shutil.which("gcc"), shutil.which("g++")
        self.assertIsNotNone(cc)
        self.assertIsNotNone(cxx)
        with tempfile.TemporaryDirectory(prefix="led-fault-test-") as name:
            temp = Path(name)
            (temp / "stm32_hal.h").write_text(HAL_SHIM)
            (temp / "pindef.h").write_text(PINDEF_SHIM)
            (temp / "FreeRTOS.h").write_text('#include "FreeRTOSConfig.h"\n')
            (temp / "task.h").write_text("typedef void *TaskHandle_t;\n")
            (temp / "harness.cpp").write_text(HARNESS)
            (temp / "caller.c").write_text(
                '#include "FreeRTOSConfig.h"\n'
                'int c_assert_uses_once(void) { int calls = 0; '
                'if (1) configASSERT(++calls); else return -1; return calls; }\n'
            )
            for device, board, family in (
                ("STM32G474xx", "STM32G474RET6", "g4"),
                ("STM32H743xx", "STM32H743ZITX", "h7"),
                ("STM32U5A9xx", "STM32U5A9NJH6Q", "u5"),
            ):
                hal = ROOT / f"Drivers/{board}/STM32{family.upper()}xx_HAL_Driver/Src/stm32{family}xx_hal.c"
                (temp / "delay.c").write_text(
                    '#include "stm32_hal.h"\n' + extract_function(hal, "void HAL_Delay(uint32_t Delay)")
                )
                flags = ["-Wall", "-Wextra", "-Werror", "-D" + device,
                         "-I" + str(temp), "-I" + str(ROOT / "Common/Drivers/Inc"),
                         "-I" + str(ROOT / "Common/Libs/Inc")]
                # Expected pins are independent of the header under test.
                default_second = {"g4": ("GPIOA", "0x20U"),
                                  "h7": ("GPIOB", "0x1U"),
                                  "u5": ("GPIOE", "0x1U")}[family]
                wirings = [("active-low", PINDEF_SHIM, [], "GPIOB", "0x200U", 0)]
                for app in ("MotorBoard", "RelayBoard", "BottomDistBoard",
                            "TopDistBoard", "TelemetryBoard"):
                    port, pin = (("GPIOB", "0x200U")
                                 if app == "TelemetryBoard" and family == "g4" else default_second)
                    mapping = (ROOT / app / "Inc/pindef.h").read_text()
                    wirings.append((app, mapping, [], port, pin, 1))
                if family == "g4":
                    # Telemetry's PB9 must also be replaced by this explicit profile.
                    wirings.append(("nucleo", (ROOT / "TelemetryBoard/Inc/pindef.h").read_text(),
                                    ["-DLED_FAULT_NUCLEO_G474"], "GPIOA", "0x20U", 1))
                for wiring, mapping, defines, port, pin, active_high in wirings:
                    (temp / "pindef.h").write_text(mapping)
                    expected = ["-DEXPECTED_SECOND_PORT=" + port,
                                "-DEXPECTED_SECOND_PIN=" + pin,
                                "-DEXPECTED_SECOND_ACTIVE_HIGH=" + str(active_high)]
                    objects = []
                    for source, compiler, standard in (
                        (ROOT / "Common/Drivers/Src/LedFault.cpp", cxx, "c++14"),
                        (temp / "harness.cpp", cxx, "c++14"),
                        (temp / "delay.c", cc, "c11"),
                        (temp / "caller.c", cc, "c11"),
                    ):
                        obj = temp / (source.stem + ".o")
                        subprocess.run([compiler, "-std=" + standard, *flags, *defines, *expected,
                                        "-c", str(source), "-o", str(obj)], check=True)
                        objects.append(str(obj))
                    binary = temp / "test"
                    subprocess.run([cxx, *objects, "-Wl,--wrap=HAL_Delay", "-o", str(binary)], check=True)
                    for case in ("normal", "hard", "stack", "malloc", "mem", "bus", "usage",
                                 "nmi", "secure", "assert", "stopped", "clockzero"):
                        with self.subTest(device=device, wiring=wiring, case=case):
                            subprocess.run([str(binary), case], check=True, timeout=5)
                print(f"{device}: HAL_Delay, LED mappings, hooks, and stopped timer passed")


if __name__ == "__main__":
    unittest.main()
