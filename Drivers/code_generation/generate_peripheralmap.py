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


def parse_XML_alternate_functions(dir, peripheral_map):
    tree = ET.parse(dir)
    root = tree.getroot()

    # Formatted as { "UART4": { "RX": { "PA0": AF#, "PA1": AF# }, "TX": { "PA2": AF#, "PA3": AF# } }, ... }
    af_map = {}
    for peripheral, map in peripheral_map.items():
        for instance, values in map.items():
            af_map[instance] = {}
            for mode, pins in values.items():
                af_map[instance][mode] = {}
                for pin in pins:
                    for pin_entry in root.findall('{http://dummy.com}GPIO_Pin'):
                        if pin_entry.attrib['Name'] == pin:
                            for signal in pin_entry.findall('{http://dummy.com}PinSignal'):
                                if signal.attrib['Name'] == instance + '_' + mode:
                                    af = signal[0][0].text
                                    af_map[instance][mode][pin] = af
    
    return af_map


def create_cpp_file(file_path, peripheral_map, pin_map, af_map):
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
        f.write("    return;\n}\n\n")


        # UART alternate function method
        f.write("uint8_t get_UART_AF(UART_HandleTypeDef* handle, Pin* pin, uint8_t mode) {\n")
        for peripheral, map in af_map.items():
            for mode, pins in map.items():
                if pins:
                    f.write(f"    AF_Info {peripheral}_{mode}[] = {{\n")
                    for pin_name, af in pins.items():
                        pin_name = pin_name[:2] + "_" + pin_name[2:]
                        f.write(f"        {{{pin_name}, {af}}},\n")
                    f.write("    };\n\n")
                    f.write(f"    uint8_t {peripheral}_{mode}_len = {len(pins)};\n\n")
        
        f.write("    AF_Info* array;\n\tuint8_t array_len;\n\n")

        for peripheral, map in af_map.items():
            f.write(f"""    if(handle == {peripheral}) {{
        if(mode == RX) {{
            array = {peripheral}_RX;
            array_len = {peripheral}_RX_len;
        }} else if(mode == TX) {{
            array = {peripheral}_TX;
            array_len = {peripheral}_TX_len;
        }}
    }}\n\n""")

        f.write("""\tfor (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == pin) {
            return array[i].AF;
        }
    }\n""")
        f.write("}\n\n")

if __name__ == "__main__":
    dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'input/STM32H743ZITx.xml')
    peripheral_map = parse_XML_peripheralmap(dir)
    pin_map = parse_XML_pinmap(dir)

    dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'input/GPIO-STM32H747_gpio_v1_0_Modes.xml')
    af_map = parse_XML_alternate_functions(dir, peripheral_map)

    file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'peripheralmap.cpp')
    create_cpp_file(file_path, peripheral_map, pin_map, af_map)

