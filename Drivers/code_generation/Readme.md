# Code Generation

Scripts in this directory are used to generate code for Rivanna3S.

## When to use this

Use this when you are adding support for a new STM32 microcontroller to the codebase. 

## Generating the pinmap and peripheralmap

Both scripts use XML files made by STM found in [this GitHub repo](https://github.com/STMicroelectronics/STM32_open_pin_data/tree/master). Download the files you need from this repo. These are the same file STM32CubeIDE uses to generate its code. 

The pinmap generations script requires 2 arguments:
- a path to the device pinmapping file. This is found under the `mcu/` folder of that repo and is the name of the device. [Example](https://github.com/STMicroelectronics/STM32_open_pin_data/blob/master/mcu/STM32H743ZITx.xml)
- the family of the microcontroller

Example running the script: `python3 ./Drivers/code_generation/generate_pinmap.py --device_pinmapping Drivers/code_generation/input/STM32H743ZITx.xml --family STM32H743`

The peripheralmap generation script requires 3 arguments:
- the same 2 from pinmap
- a path to the family peripheral list file. This is found under the `mcu/IP/` folder of that repo. The naming of the file is not always consistent, but usually similar to `GPIO-STM32H747_gpio_v1_0_Modes.xml`. [Example](https://github.com/STMicroelectronics/STM32_open_pin_data/blob/master/mcu/IP/GPIO-STM32H747_gpio_v1_0_Modes.xml)

Example running the script: `python ./Drivers/code_generation/generate_peripheralmap.py --device_pinmapping Drivers/code_generation/input/STM32H743ZITx.xml --family stm32h7 --family_peripheral_list ./Drivers/code_generation/input/GPIO-STM32H747_gpio_v1_0_Modes.xml`