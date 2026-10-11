# Debug LED fault signaling

`Common/Drivers/Inc/LedFault.h` and `Common/Drivers/Src/LedFault.cpp` route CPU
faults, FreeRTOS assertions, allocation failures, and stack overflows into a
terminal LED signal. The driver is compiled separately for each application,
so its `pindef.h` supplies the GPIO ports, pin masks, and polarity. It uses no
Thread, Clock, DigitalOut, logging, allocation, or scheduler calls in the fault
path. Both FreeRTOS hooks have C linkage; the stack hook ignores its arguments.

## Hardware mappings and assumptions

[Issue #204's comment](https://github.com/solarcaratuva/Rivanna3S/issues/204#issuecomment-5860390277)
and its linked [V3 pinouts](https://docs.google.com/document/d/1uve1M_h-JJsi3HeXx6rrDRb_oYeCWjquN5Ofh0h0lCM/edit)
identify **PC1 as Debug_OUT** on STM boards. PC1 uses `GPIO_PIN_1` (`0x0002`),
not the integer `1`, which selects PC0. PC13 is labeled Debug_IN.

The supplied *internal led blink.pdf*, pages 2 and 9, demonstrates the Nucleo's
active-high **PA5** LED and direct GPIO register writes. ST's
[UM2505, section 7.6](https://www.st.com/resource/en/user_manual/um2505-stm32g4-nucleo64-boards-mb1367-stmicroelectronics.pdf)
confirms that NUCLEO-G474RE LD2 is on PA5 with solder bridge SB6 closed, and
**PC13 is the user button**, not another LED. The Nucleo has only one user LED;
its power and ST-LINK LEDs are not a second target-controlled fault output.
The team's [HIL bench plan](https://github.com/solarcaratuva/Rivanna3S/issues/314)
specifies a NUCLEO-G474RE. This establishes PA5 for that bench, but does not
establish it for bare-MCU custom boards.

The remaining mappings use the user's requested best guesses. H7 uses PB0,
the green LED on [NUCLEO-H743ZI, UM1974](https://www.st.com/resource/en/user_manual/um1974-stm32h743-nucleo144-board-stmicroelectronics.pdf).
U5 uses PE0, the green LED defined by ST's
[STM32U5x9J-DK BSP](https://github.com/STMicroelectronics/stm32u5x9j-dk-bsp/blob/main/stm32u5x9j_discovery.h);
its [LED-on implementation](https://github.com/STMicroelectronics/stm32u5x9j-dk-bsp/blob/main/stm32u5x9j_discovery.c)
sets that GPIO high. These are development-board defaults, not confirmation
of the car PCBs' fitted circuits. All mappings assume active-high LEDs.

Select the fitted hardware separately from the MCU with `LED_FAULT_HARDWARE`:

| Configuration | First output | Second output |
| --- | --- | --- |
| `BOARD`, Motor/Relay/BottomDist/TopDist, G4 | PC1 Debug_OUT | PA5 (G474 Nucleo default) |
| `BOARD`, Motor/Relay/BottomDist/TopDist, H7 | PC1 Debug_OUT | PB0 (H743 Nucleo default) |
| `BOARD`, Motor/Relay/BottomDist/TopDist, U5 | PC1 Debug_OUT | PE0 (U5x9J-DK default) |
| `BOARD`, Telemetry | PC1 Debug_OUT | Existing PB9 DEBUG_LED_2 |
| `NUCLEO_G474`, any application | PC1 external/carrier debug LED | PA5 onboard LD2 |

The configured outputs currently use active-high polarity. Each board's
`FAULT_LED_*_ACTIVE_HIGH` can be changed for a verified active-low circuit.
On a standalone Nucleo, add an active-high LED with a series resistor from PC1
to GND for the second physical light. PC1 is available on CN7 pin 36 with the
default connector solder bridges (SB35 closed, SB34 open). Verify the carrier
does not reroute that connector pin. The Nucleo profile requires
`BOARD=STM32G474RET6`; it is not inferred for H7 or U5.

Every build configures two outputs; no second LED is silently disabled. For a
different fitted circuit, change `FAULT_LED_2_PORT`, `FAULT_LED_2_PIN`, and
`FAULT_LED_2_ACTIVE_HIGH` in the corresponding board/MCU branch of `pindef.h`.
The primary PC1 polarity is configurable there too. On a bare-MCU board, fit
the second LED on the selected output or replace that mapping. **Both LEDs'
physical wiring and polarity still require bench verification.**

## Patterns and timing

The low nibble counts slow pulses; the high nibble counts fast pulses. All
configured LEDs blink together. Slow pulses use 500 ms ON/OFF, fast pulses use
200 ms ON/OFF, followed by a further 1500 ms OFF pause.

| Fault | Pattern |
| --- | --- |
| HardFault | 1 slow pulse |
| Stack overflow | 2 slow pulses |
| Allocation failure | 3 slow pulses |
| MemManage | 4 slow pulses |
| BusFault | 5 slow pulses |
| UsageFault | 6 slow pulses |
| NMI | 7 slow pulses |
| SecureFault (U5) | 8 slow pulses |
| FreeRTOS assertion | 1 slow pulse, then 1 fast pulse |

The terminal handler disables interrupts, enables each GPIO clock, preloads
the ON state, and configures only those pins as push-pull outputs. All LEDs
are ON before timer setup or the first delay. GPIO writes use BSRR directly.
G4, H7, and U5 CPU exception handlers all call this shared terminal handler.

Every delay calls the **unmodified vendor `HAL_Delay()`**. Its usual
interrupt-driven tick cannot advance inside HardFault or a critical fault
hook. The terminal handler therefore takes over SysTick with a 1 ms reload
based on `SystemCoreClock`, leaves its interrupt disabled, and polls its
COUNTFLAG through a strong `HAL_GetTick()` override. Before a terminal fault,
`HAL_GetTick()` returns the usual `uwTick` and leaves SysTick alone. Normal
execution never resumes after a fault; reset restores the usual time base.
If the hardware timer cannot run, the first delay stalls with the LEDs ON.

## Automated validation

Run `python3 tests/test_led_fault.py`. It compiles the real C++ driver and a C
assertion caller, and uses the actual `HAL_Delay()` body from each checked-in
G4, H7, and U5 HAL. Simulated registers verify two different GPIO ports and
polarities, every board/MCU mapping, the G474 Nucleo override, all patterns,
hooks, the unchanged normal tick, interrupt-free fault timing, preservation
of neighboring pins, and the stopped-timer fallback.
These checks do not establish that LEDs are physically fitted.

The G4 configuration builds all five board targets. H7 has existing application
pinmap and FATFS include failures; U5 lacks generated `pinmap.h` and
`peripheralmap.cpp`, and its FreeRTOS port is absent. Changed fault sources can
be compiled independently against each family's actual HAL/CMSIS headers, but
that does not certify complete H7/U5 firmware.

## Bench firmware

`LED_FAULT_BENCH_TEST` defaults to `OFF`. An explicit `HARDFAULT` or
`STACKOVERFLOW` build replaces each selected application's sources with
`tests/firmware/LedFaultBench.cpp`; normal application actuator tasks are not
started. Use separate build directories so injection cannot accidentally
remain enabled in a normal build.

```sh
cmake -S . -B build-fault-hard -G Ninja \
  -DBOARD=STM32G474RET6 -DLED_FAULT_HARDWARE=NUCLEO_G474 \
  -DLED_FAULT_BENCH_TEST=HARDFAULT
cmake --build build-fault-hard --target MotorBoard.elf

cmake -S . -B build-fault-stack -G Ninja \
  -DBOARD=STM32G474RET6 -DLED_FAULT_HARDWARE=NUCLEO_G474 \
  -DLED_FAULT_BENCH_TEST=STACKOVERFLOW
cmake --build build-fault-stack --target MotorBoard.elf
```

Run these commands in the project's ARM GCC build environment. The artifacts
are `build-fault-hard/bin/MotorBoard.elf` and
`build-fault-stack/bin/MotorBoard.elf`. Flash the chosen artifact through the
usual debugger/CubeProgrammer tooling on a disconnected bench board.

HardFault injection executes an undefined instruction with UsageFault disabled,
so it escalates to the actual HardFault callback. Stack injection creates a
128-word static task stack inside a larger reserved buffer and recursively
uses more than the registered stack, overwriting FreeRTOS's stack sentinels
without writing outside the reserved buffer. At the next context switch,
FreeRTOS stack checking invokes the real overflow hook. Recursion is bounded
and cannot be optimized into a tail call.

`led_fault_bench_stage` is available in the debugger: 0 means waiting, 1 means
injection started, 2 means stack recursion returned, and 3 means the expected
terminal hook was not reached.

## Physical checks still required

No physical fault injection has been performed. The local probe inventory
reported no ST-LINK. On the bench:

1. Fit/check both LEDs and their polarity; leave actuator/HV connections
   disconnected. If using a standalone Nucleo, fit the PC1 external LED.
2. Flash the HardFault artifact, continue past any debugger exception catch,
   and confirm both LEDs repeat one slow pulse.
3. Reset and flash the stack artifact; confirm the actual overflow hook is
   reached and both LEDs repeat two slow pulses. Confirm stage 3 is not reached.
4. Stop SysTick at the first delay in a fault and confirm both LEDs stay ON.
   Repeat with a fault before application GPIO initialization.
5. Restore a normal `LED_FAULT_BENCH_TEST=OFF` build before reconnecting the
   board. Record the hardware model, LED wiring/polarity, observed patterns,
   and test results in the review evidence.

The software now implements two outputs and all required fault hooks with
documented wiring assumptions. Issue #204's physical two-LED behavior and
bench-testing requirements remain unverified until these checks pass. The
issue itself is not modified by this implementation.
