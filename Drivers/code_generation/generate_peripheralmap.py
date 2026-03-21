import xml.etree.ElementTree as ET
import os
import re
import argparse
from generate_pinmap import parse_XML_pinmap

""" 
TODO: 
    - Generate SPI peripheral array and get_SPI_AF function
"""

# Define specific pins required for each peripheral type
SIGNAL_MAP = {
    "UART": ["RX", "TX"],
    "USART": ["RX", "TX"],
    "I2C": ["SDA", "SCL"],
    "SPI": ["MOSI", "MISO", "SCK"],
    "FDCAN": ["RX", "TX"],
    "ADC": None
}


def parse_XML_peripheralmap(dir: str) -> dict:
    tree = ET.parse(dir)
    root = tree.getroot()

    peripheral_map = {peripheral: {} for peripheral in SIGNAL_MAP.keys()}

    # Get all peripherals
    for peripheral in root.findall('{http://dummy.com}IP'):
        if peripheral.attrib['Name'] in peripheral_map:
            peripheral_map[peripheral.attrib['Name']][peripheral.attrib['InstanceName']] = {}

   
    pattern = r"^P[A-Z](?:\d{1}|\d{2})$"

    # Fill in pin mappings
    for pin in root.findall('{http://dummy.com}Pin'):
        pin_name = pin.attrib['Name']
        position = int(pin.attrib['Position'])

        if not re.match(pattern, pin_name) or position > 64:
            continue

        # Check each signal in this pin
        for signal in pin.findall('{http://dummy.com}Signal'):
            signal_name = signal.attrib['Name']

            # Split into instance + signal type
            if "_" not in signal_name:
                continue
            
            instance, sig_type = signal_name.split("_", 1)

            # Find which peripheral this instance belongs to
            for peripheral, instances in peripheral_map.items():
                if instance not in instances:
                    continue

                # Handle ADC channels separately
                if peripheral == "ADC":
                    if sig_type.startswith("INP"):
                        channel = int(sig_type[3:])
                        instances[instance].setdefault(channel, []).append(pin_name)

                # For other peripherals, check if signal type is valid
                else:
                    valid_signals = SIGNAL_MAP[peripheral]
                    if sig_type in valid_signals:
                        instances[instance].setdefault(sig_type, []).append(pin_name)
                    
    return peripheral_map


def parse_XML_alternate_functions(dir: str, peripheral_map: dict) -> dict:
    tree = ET.parse(dir)
    root = tree.getroot()

    # Formatted as { "UART4": { "RX": { "PA0": AF#, "PA1": AF# }, "TX": { "PA2": AF#, "PA3": AF# } }, ... }
    af_map = {}
    for peripheral, map in peripheral_map.items():
        # Skip ADC as it doesn't use alternate functions
        if peripheral.startswith("ADC"):
            continue
        af_map[peripheral] = {}
        for instance, values in map.items():
            af_map[peripheral][instance] = {}
            for mode, pins in values.items():
                af_map[peripheral][instance][mode] = {}
                for pin in pins:
                    for pin_entry in root.findall('{http://dummy.com}GPIO_Pin'):
                        if pin_entry.attrib['Name'] == pin:
                            for signal in pin_entry.findall('{http://dummy.com}PinSignal'):
                                if signal.attrib['Name'] == instance + '_' + mode:
                                    af = signal[0][0].text
                                    af_map[peripheral][instance][mode][pin] = af
    
    return af_map


def write_af_arrays(f, af_map: dict, peripheral_type: str) -> None:
    for peripheral, modes in af_map[peripheral_type].items():
        for mode, pins in modes.items():
            if not pins:
                continue

            f.write(f"    static AF_Info {peripheral}_{mode}[] = {{\n")
            for pin_name, af in pins.items():
                c_pin = pin_name[:2] + "_" + pin_name[2:]
                f.write(f"        {{{c_pin}, {af}}},\n")
            f.write("    };\n")
            f.write(f"    static uint8_t {peripheral}_{mode}_len = {len(pins)};\n\n")


def write_get_uart_af(f, af_map: dict) -> None:
    f.write("uint8_t get_UART_AF(const USART_TypeDef* handle, const Pin* pin, uint8_t mode) {\n")

    write_af_arrays(f, af_map, "UART")
    write_af_arrays(f, af_map, "USART")

    f.write("    AF_Info* array = NULL;\n    uint8_t array_len = 0;\n\n")

    for peripheral in af_map.keys():
        if peripheral == "UART" or peripheral == "USART":
            for instance in af_map[peripheral].keys():
                if af_map[peripheral][instance]:
                    f.write(f"""
    if(handle == {instance}) {{
        if(mode == RX) {{
            array = {instance}_RX;
            array_len = {instance}_RX_len;
        }} else if(mode == TX) {{
            array = {instance}_TX;
            array_len = {instance}_TX_len;
        }}
    }}
""")

    f.write("""
    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}\n\n""")


def write_get_i2c_af(f, af_map: dict) -> None:
    f.write("uint8_t get_I2C_AF(const I2C_TypeDef* handle, const Pin* pin, uint8_t mode) {\n")

    write_af_arrays(f, af_map, "I2C")

    f.write("    AF_Info* array = NULL;\n    uint8_t array_len = 0;\n\n")

    for peripheral in af_map['I2C'].keys():
        if af_map['I2C'][peripheral]:
            f.write(f"""
    if(handle == {peripheral}) {{
        if(mode == SDA) {{
            array = {peripheral}_SDA;
            array_len = {peripheral}_SDA_len;
        }} else if(mode == SCL) {{
            array = {peripheral}_SCL;
            array_len = {peripheral}_SCL_len;
        }}
    }}
""")

    f.write("""
    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}\n\n""")


def write_get_fdcan_af(f, af_map: dict) -> None:
    f.write("uint8_t get_FDCAN_AF(const FDCAN_GlobalTypeDef* handle, const Pin* pin, uint8_t mode) {\n")

    write_af_arrays(f, af_map, "FDCAN")

    f.write("    AF_Info* array = NULL;\n    uint8_t array_len = 0;\n\n")

    for peripheral in af_map['FDCAN'].keys():
        if af_map['FDCAN'][peripheral]:
            f.write(f"""
    if(handle == {peripheral}) {{
        if(mode == RX) {{
            array = {peripheral}_RX;
            array_len = {peripheral}_RX_len;
        }} else if(mode == TX) {{
            array = {peripheral}_TX;
            array_len = {peripheral}_TX_len;
        }}
    }}
""")

    f.write("""
    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}\n\n""")


def write_uart_array(f, peripheral_map: dict) -> None:
    f.write("UART_Peripheral UART_Peripherals[] = {\n")

    for bus in ("UART", "USART"):
        for instance, values in peripheral_map.get(bus, {}).items():
            if not values:
                continue
            if not values["RX"] or not values["TX"]:
                continue

            rx_mask = " | ".join(pin[:2] + "_" + pin[2:] + ".universal_mask" for pin in values["RX"])
            tx_mask = " | ".join(pin[:2] + "_" + pin[2:] + ".universal_mask" for pin in values["TX"])

            f.write(
                f"    {{{instance}, ({rx_mask}), ({tx_mask}), NC, NC, false}},\n"
            )

    f.write("};\n\n")
    f.write("const uint8_t UART_PERIPHERAL_COUNT = sizeof(UART_Peripherals) / sizeof(UART_Peripherals[0]);\n\n")


def write_i2c_array(f, peripheral_map: dict) -> None:
    f.write("I2C_Peripheral I2C_Peripherals[] = {\n")

    for instance, values in peripheral_map.get("I2C", {}).items():
        if not values:
            continue
        if not values["SDA"] or not values["SCL"]:
            continue

        sda_mask = " | ".join(pin[:2] + "_" + pin[2:] + ".universal_mask" for pin in values["SDA"])
        scl_mask = " | ".join(pin[:2] + "_" + pin[2:] + ".universal_mask" for pin in values["SCL"])

        f.write(
            f"    {{{instance}, ({sda_mask}), ({scl_mask}), NC, NC, false}},\n"
        )

    f.write("};\n\n")
    f.write("const uint8_t I2C_PERIPHERAL_COUNT = sizeof(I2C_Peripherals) / sizeof(I2C_Peripherals[0]);\n\n")


def write_fdcan_array(f, peripheral_map: dict) -> None:
    f.write("FDCAN_Peripheral FDCAN_Peripherals[] = {\n")

    for instance, values in peripheral_map.get("FDCAN", {}).items():
        if not values:
            continue
        if not values["RX"] or not values["TX"]:
            continue

        rxd_mask = " | ".join(pin[:2] + "_" + pin[2:] + ".universal_mask" for pin in values["RX"])
        txd_mask = " | ".join(pin[:2] + "_" + pin[2:] + ".universal_mask" for pin in values["TX"])

        f.write(
            f"    {{{instance}, ({rxd_mask}), ({txd_mask}), NC, NC, false}},\n"
        )

    f.write("};\n\n")
    f.write("const uint8_t FDCAN_PERIPHERAL_COUNT = sizeof(FDCAN_Peripherals) / sizeof(FDCAN_Peripherals[0]);\n\n")


def write_adc_array(f, peripheral_map: dict) -> None:
    adc_map = peripheral_map.get("ADC", {})

    # Assign instance numbers in consistent order
    adc_instances = list(adc_map.keys())
    instance_to_num = {adc: idx for idx, adc in enumerate(adc_instances)}

    f.write("ADC_Peripheral ADC_Peripherals[] = {\n")

    for instance, channels in adc_map.items():
        instance_num = instance_to_num[instance]

        for channel, pins in channels.items():
            for pin in pins:
                port = pin[:2]
                num = pin[2:]

                pin_mask = f"{port}_{num}.universal_mask"

                f.write(
                    f"    {{{instance}, ADC_CHANNEL_{channel}, {pin_mask}, NC, false, {instance_num}}},\n"
                )

    f.write("};\n\n")

    f.write(
        "const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / "
        "sizeof(ADC_Peripherals[0]);\n\n"
    )

    # adc_channels_claimed array (all zeros)
    f.write(
        f"uint8_t adc_channels_claimed[] = "
        f"{{{', '.join(['0'] * len(adc_instances))}}};\n\n"
    )

def create_cpp_file(file_path: str, peripheral_map: dict, pin_map: dict, af_map: dict, family: str) -> None:
    with open(file_path, 'w') as f:
        # Includes
        f.write(f'#include "peripheralmap.h"\n#include "{family.lower()}xx_hal.h"\n\n')

        # Peripheral arrays
        write_uart_array(f, peripheral_map)
        write_i2c_array(f, peripheral_map)
        write_fdcan_array(f, peripheral_map)
        write_adc_array(f, peripheral_map)

        # GPIO clock enable function
        f.write("void gpio_clock_enable(const GPIO_TypeDef* handle){\n")
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

        # Alternate function methods
        write_get_uart_af(f, af_map)
        write_get_i2c_af(f, af_map)
        write_get_fdcan_af(f, af_map)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate peripheralmap source file from XML",)
    parser.add_argument("--device_pinmapping", help="Path to the device pinmapping input XML file", required=True)
    parser.add_argument("--family_peripheral_list", help="Path to the family peripheral list input XML file", required=True)
    parser.add_argument("--family", help="Name of the device family (e.g. STM32H743)", required=True)

    args = parser.parse_args()

    peripheral_map = parse_XML_peripheralmap(args.device_pinmapping)
    pin_map = parse_XML_pinmap(args.device_pinmapping)
    af_map = parse_XML_alternate_functions(args.family_peripheral_list, peripheral_map)

    output_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'peripheralmap.cpp')
    create_cpp_file(output_file_path, peripheral_map, pin_map, af_map, args.family)


if __name__ == "__main__":
    main()