import xml.etree.ElementTree as ET
import os
import re
from generate_pinmap import parse_XML_pinmap

""" 
Only need these peripherals: I2C, SPI (and QuadSPI), UART (and USART), CAN (they call it FDCAN), ADC

Currently only implementing UART and USART.

TODO: 
    - Update header file and cpp file to include other peripherals as well.
    - Add alternate function for UART peripherals.
"""

def parse_XML_peripheralmap(dir):
    tree = ET.parse(dir)
    root = tree.getroot()

    peripheral_map = {"UART": {}, "USART": {}}

    # Get all peripherals
    for peripheral in root.findall('{http://dummy.com}IP'):
        if peripheral.attrib['Name'] in peripheral_map:
            peripheral_map[peripheral.attrib['Name']][peripheral.attrib['InstanceName']] = {}

    # Find valid RX and TX pins for UART and USART peripherals
    for peripheral, map in peripheral_map.items():
        for instance, values in map.items():
            values['RX'] = []
            values['TX'] = []
            for pin in root.findall('{http://dummy.com}Pin'):
                pin_name = pin.attrib['Name']
                position = int(pin.attrib['Position'])

                pattern = r"^P[A-Z](?:\d{1}|\d{2})$"

                if re.match(pattern, pin_name) and position <= 64:
                    for signal in pin.findall('{http://dummy.com}Signal'):   
                        if signal.attrib['Name'] == instance + '_RX':
                            values['RX'].append(pin_name)
                        if signal.attrib['Name'] == instance + '_TX':
                            values['TX'].append(pin_name)
                    
    return peripheral_map


def create_header_file(file_path):
    with open(file_path, 'w') as f:
        # Header guard and includes
        f.write("""#ifndef PERIPHERALMAP_H
#define PERIPHERALMAP_H\n
#include \"stm32h743xx.h\"
#include \"pinmap.h\"
#include <stdbool.h>\n""")

        # UART struct definition
        f.write(""" 
// UART peripheral struct
typedef struct {
    USART_TypeDef* handle;
    uint64_t rxd_valid_pins;
    uint64_t txd_valid_pins;
    Pin rxd_used;
    Pin txd_used;
    uint8_t alternate_function;
    bool isClaimed;
    // Queue[float] return_value_queue
} UART_Peripheral;\n""")

        # Global arrays
        f.write(""" 
// Declare global arrays
extern UART_Peripheral UART_Peripherals[];
extern const uint8_t UART_PERIPHERAL_COUNT;\n""")
        
        # Clock enable functions
        f.write("""
void uart_clock_enable(USART_TypeDef* handle);\n
void gpio_clock_enable(GPIO_TypeDef* handle);\n\n""")

        # End header guard
        f.write("#endif /* PERIPHERALMAP */")


def create_cpp_file(file_path, peripheral_map, pin_map):
    with open(file_path, 'w') as f:
        # Includes
        f.write('#include "peripheralmap.h"\n#include "stm32h7xx_hal.h"\n\n')

        # Array of UART peripherals
        f.write("UART_Peripheral UART_Peripherals[] = {\n")
        for peripheral, map in peripheral_map.items():
            for instance, values in map.items():
                if values['RX'] and values['TX']:
                    f.write(f"    {{{instance}, (")
                    for index, pin in enumerate(values['RX']):
                        pin_name = pin[:2] + "_" + pin[2:]
                        if index == len(values['RX']) - 1:
                            f.write(f"{pin_name}.universal_mask), ")
                        else:
                            f.write(f"{pin_name}.universal_mask | ")
                    
                    f.write("(")
                    for index, pin in enumerate(values['TX']):
                        pin_name = pin[:2] + "_" + pin[2:]
                        if index == len(values['TX']) - 1:
                            f.write(f"{pin_name}.universal_mask), ")
                        else:
                            f.write(f"{pin_name}.universal_mask | ")
                    f.write("NC, NC, false},\n")
        f.write("};\n\n")

        f.write("const uint8_t UART_PERIPHERAL_COUNT = sizeof(UART_Peripherals) / sizeof(UART_Peripherals[0]);\n\n")


        # UART clock enable function
        f.write("void uart_clock_enable(USART_TypeDef* handle){\n")
        peripherals = list(peripheral_map["UART"].keys())
        peripherals.extend(list(peripheral_map["USART"].keys()))

        for index, instance in enumerate(peripherals):
            if index == 0:
                f.write(f"    if(handle == {instance}){{\n        __HAL_RCC_{instance}_CLK_ENABLE();\n    }}\n")
            else:
                f.write(f"    else if(handle == {instance}){{\n        __HAL_RCC_{instance}_CLK_ENABLE();\n    }}\n")
        f.write("    return;\n}\n\n")


        # GPIO clock enable function
        f.write("void gpio_clock_enable(GPIO_TypeDef* handle){\n")
        gpio_blocks = set()
        for pin_name in pin_map.keys():
            block = pin_name[1]
            gpio_blocks.add(block)

        gpio_blocks = list(gpio_blocks)
        for index, block in enumerate(gpio_blocks):
            if index == 0:
                f.write(f"    if(handle == GPIO{block}){{\n        __HAL_RCC_GPIO{block}_CLK_ENABLE();\n    }}\n")
            else:
                f.write(f"    else if(handle == GPIO{block}){{\n        __HAL_RCC_GPIO{block}_CLK_ENABLE();\n    }}\n")
        f.write("    return;\n}\n")
        

if __name__ == "__main__":    
    file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'peripheralmap.h')
    create_header_file(file_path)

    dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'input/STM32H743ZITx.xml')
    peripheral_map = parse_XML_peripheralmap(dir)
    pin_map = parse_XML_pinmap(dir)

    file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'peripheralmap.cpp')
    create_cpp_file(file_path, peripheral_map, pin_map)

