#!/usr/bin/env python3

import argparse
import re
from pathlib import Path

CHANGE_COMMENT = " // <-- This line changed when importing"

# helpers

def regex_replace_once(text: str, pattern: str, replacement: str) -> str:
    def _repl(match: re.Match) -> str:
        return match.expand(replacement) + CHANGE_COMMENT

    return re.sub(pattern, _repl, text, count=1, flags=re.MULTILINE)

def regex_replace_all(text: str, pattern: str, replacement: str) -> str:
    def _repl(match: re.Match) -> str:
        return match.expand(replacement) + CHANGE_COMMENT

    return re.sub(pattern, _repl, text, count=0, flags=re.MULTILINE)

def remove_function_by_name(text: str, func_name: str) -> str:
    pattern = re.compile(
        rf'void\s+{func_name}\s*\([^)]*\)\s*\{{',
        re.MULTILINE
    )

    match = pattern.search(text)
    if not match:
        return text

    start = match.start()
    i = match.end()

    brace_depth = 1
    while i < len(text) and brace_depth > 0:
        if text[i] == '{':
            brace_depth += 1
        elif text[i] == '}':
            brace_depth -= 1
        i += 1

    return text[:start] + text[i:]

def insert_adc_dispatcher(text: str) -> str:
    adc_nums = sorted(set(re.findall(r'void\s+MX_ADC(\d+)_Init', text)))
    if not adc_nums:
        return text

    last_adc = adc_nums[-1]

    insert_pattern = re.compile(
        rf'void\s+MX_ADC{last_adc}_Init[^\{{]*\{{',
        re.MULTILINE
    )

    match = insert_pattern.search(text)
    if not match:
        return text

    i = match.end()
    brace_depth = 1
    while i < len(text) and brace_depth > 0:
        if text[i] == '{':
            brace_depth += 1
        elif text[i] == '}':
            brace_depth -= 1
        i += 1

    func = (
        "\nADC_HandleTypeDef* ADC_init(const ADC_TypeDef* hadc, uint32_t channel, uint32_t rank) {\n"
    )

    for idx, n in enumerate(adc_nums):
        if idx == 0:
            func += f"  if (hadc == ADC{n}) {{\n"
        else:
            func += f"  else if (hadc == ADC{n}) {{\n"

        func += (
            f"    MX_ADC{n}_Init(channel, rank);\n"
            f"    return NULL;\n"
            "  }\n"
        )

    func += (
        "\n  return &hadc1; // Default return to avoid compiler warning\n"
        "}\n"
    )

    return text[:i] + func + text[i:]

def insert_uart_dispatcher(text: str) -> str:
    import re

    # Find all MX_UARTx_Init / MX_USARTx_UART_Init functions
    uart_matches = re.findall(
        r'\b(MX_(?:UART\d+|USART\d+_UART)_Init)\s*\(',
        text
    )
    if not uart_matches:
        return text

    # Deduplicate while preserving order
    seen = set()
    uart_inits = []
    for m in uart_matches:
        if m not in seen:
            seen.add(m)
            uart_inits.append(m)

    func = (
        "\n\nUART_HandleTypeDef* UART_init(const USART_TypeDef* uart, uint32_t baudrate) {\n"
        "    UART_HandleTypeDef* handle;\n\n"
    )

    for idx, init in enumerate(uart_inits):
        # Strip MX_ and _Init
        base = init.replace("MX_", "").replace("_Init", "")

        # Extract UART/USART number
        num_match = re.search(r'(\d+)', base)
        if not num_match:
            continue

        num = num_match.group(1)

        # Peripheral instance name
        if base.startswith("USART"):
            instance = f"USART{num}"
        else:
            instance = f"UART{num}"

        handle = f"huart{num}"

        if idx == 0:
            func += f"    if (uart == {instance}) {{\n"
        else:
            func += f"    else if (uart == {instance}) {{\n"

        func += (
            f"        {init}(baudrate);\n"
            f"        handle = &{handle};\n"
            "    }\n"
        )

    func += (
        "    else {\n"
        "        // Unsupported UART instance\n"
        "        return NULL;\n"
        "    }\n\n"
        "    return handle;\n"
        "}\n"
    )

    return text.rstrip() + func

def insert_fdcan_dispatcher(text: str) -> str:
    import re

    # Find all MX_FDCANx_Init functions
    fdcan_matches = re.findall(r'\b(MX_(FDCAN\d+)_Init)\s*\(', text)

    # Deduplicate while preserving order
    seen = set()
    fdcan_inits = []
    for full, instance in fdcan_matches:
        if full not in seen:
            seen.add(full)
            fdcan_inits.append((full, instance))

    # Start dispatcher function
    func = (
        "\n\nFDCAN_HandleTypeDef* FDCAN_init(FDCAN_GlobalTypeDef *hadc) {\n"
    )

    for idx, (init_name, instance_name) in enumerate(fdcan_inits):
        if idx == 0:
            func += f"    if (hadc == {instance_name}) {{\n"
        else:
            func += f"    else if (hadc == {instance_name}) {{\n"

        func += f"        {init_name}(baudrate);\n"
        func += f"        return &hfdcan{instance_name[-1]};\n"
        func += "    }\n"

    func += (
        "    else {\n"
        "        // Unsupported FDCAN instance\n"
        f"        return &hfdcan{fdcan_inits[0][1][-1]};\n"
        "    }\n"
        "}\n"
    )

    return text.rstrip() + func

def insert_spi_dispatcher(text: str) -> str:
    import re

    # Find all MX_FDCANx_Init functions
    spi_matches = re.findall(r'\b(MX_(SPI\d+)_Init)\s*\(', text)

    # Deduplicate while preserving order
    seen = set()
    spi_inits = []
    for full, instance in spi_matches:
        if full not in seen:
            seen.add(full)
            spi_inits.append((full, instance))

    # Start dispatcher function
    func = (
        "\n\nSPI_HandleTypeDef* SPI_init(SPI_TypeDef *spiHandle, uint32_t baudrate_prescaler) {\n"
    )

    for idx, (init_name, instance_name) in enumerate(spi_inits):
        if idx == 0:
            func += f"    if (spiHandle == {instance_name}) {{\n"
        else:
            func += f"    else if (spiHandle == {instance_name}) {{\n"

        func += f"        {init_name}(baudrate_prescaler);\n"
        func += f"        return &hspi{instance_name[-1]};\n"
        func += "    }\n"

    func += (
        "    else {\n"
        "        // Unsupported SPI instance\n"
        f"        return &hspi{spi_inits[0][1][-1]};\n"
        "    }\n"
        "}\n"
    )

    return text.rstrip() + func

# per-file fixes

def fix_adc_c(text: str) -> str:
    # fix include
    include_pattern = r'^\s*#include\s+"adc\.h"\s*$'
    include_replacement = (
        '#include "stm32h7xx_hal.h"\n'
        '#include "pinmap.h"\n'
        '#include "peripheralmap.h"\n'
    )

    text = regex_replace_once(text, include_pattern, include_replacement)

    # comment out error handler
    initname_pattern = r'Error_Handler\(\);'
    initname_replacement = r'//Error_Handler();'

    text = regex_replace_all(text, initname_pattern, initname_replacement)

    # fix ADC resolution
    resolution_pattern = r'ADC_RESOLUTION_\d+B'
    resolution_replacement = r'ADC_RESOLUTION_12B'
    text = regex_replace_all(text, resolution_pattern, resolution_replacement)

    # fix ADC_init
    pattern = r'void\s+MX_ADC(\d+)_Init\s*\(\s*void\s*\)'
    replacement = r'void MX_ADC\1_Init(uint32_t channel, uint32_t rank)'

    text = regex_replace_all(text, pattern, replacement)

    # fix channels
    pattern = r'^\s*sConfig\.Channel\s*=.*?;'
    replacement = r'    sConfig.Channel = channel;'

    text = regex_replace_all(text, pattern, replacement)

    # fix ranks
    pattern = r'^\s*sConfig\.Rank\s*=.*?;'
    replacement = r'sConfig.Rank = rank;'

    text = regex_replace_all(text, pattern, replacement)

    # fix name for mspInit
    text = regex_replace_once(
        text,
        r'void\s+HAL_ADC_MspInit\s*\(\s*ADC_HandleTypeDef\s*\*\s*adcHandle\s*\)',
        'void HAL_ADC_MspInit_custom(const ADC_TypeDef* adcHandle, Pin pin)'
    )

    # adcHandle->Instance == ADCx  -> adcHandle == ADCx
    text = regex_replace_all(
        text,
        r'adcHandle\s*->\s*Instance\s*==\s*(ADC\d+)',
        r'adcHandle==\1)'
    )

    # GPIO_InitStruct.Pin = pin.block_mask
    text = regex_replace_all(
        text,
        r'^\s*GPIO_InitStruct\.Pin\s*=.*?;',
        '    GPIO_InitStruct.Pin = pin.block_mask;'
    )

    # fix GPIO init name
    text = regex_replace_all(
        text,
        r'HAL_GPIO_Init\s*\(\s*GPIO[A-Z]+\s*,\s*&GPIO_InitStruct\s*\)\s*;',
        '    HAL_GPIO_Init(pin.block, &GPIO_InitStruct);'
    )

    # remove deinit
    text = remove_function_by_name(text, "HAL_ADC_MspDeInit")

    # insert adc_dispatcher
    text = insert_adc_dispatcher(text)

    return text

def fix_usart_c(text: str) -> str:
    # fix include
    include_pattern = r'^\s*#include\s+"usart\.h"\s*$'
    include_replacement = (
        '#include "stm32h7xx_hal.h"\n'
        '#include "pinmap.h"\n'
        '#include "peripheralmap.h"\n'
    )

    text = regex_replace_all(text,include_pattern,include_replacement)

    # comment out error handler
    text = regex_replace_all(
        text,
        r'Error_Handler\(\);',
        '//Error_Handler();'
    )

    # fix UART init signature
    text = regex_replace_all(
        text,
        r'void\s+(MX_.*_Init)\s*\(\s*void\s*\)',
        r'void \1(uint32_t baudrate)'
    )

    # replace .Init.BaudRate = <any>;
    text = regex_replace_all(
        text,
        r'\.Init\.BaudRate\s*=\s*\d+;',
        '.Init.BaudRate = baudrate;'
    )

    # replace HAL_UART_MspInit
    text = regex_replace_once(
        text,
        r'void\s+HAL_UART_MspInit\s*\(\s*UART_HandleTypeDef\s*\*\s*uartHandle\s*\)',
        'void HAL_UART_MspInit_custom(const USART_TypeDef* uartHandle, Pin pin, uint8_t af)'
    )

    # replace uartHandle->Instance -> uartHandle
    text = regex_replace_all(
        text,
        r'uartHandle\s*->\s*Instance\s*==\s*([A-Za-z_]\w*\d+)',
        r'uartHandle == \1)'
    )

    # GPIO pin
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Pin\s*=\s*).*?;',
        r'\1pin.block_mask;'
    )

    # GPIO init
    text = regex_replace_all(
        text,
        r'HAL_GPIO_Init\s*\(\s*[A-Z0-9_]+,\s*&GPIO_InitStruct\s*\);',
        'HAL_GPIO_Init(pin.block, &GPIO_InitStruct);'
    )

    # af
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Alternate\s*=\s*).*?;',
        r'\1af;'
    )

    # remove HAL_UART_MspDeInit
    text = remove_function_by_name(text, "HAL_UART_MspDeInit")

    # insert dispatcher
    text = insert_uart_dispatcher(text)

    return text

def fix_fdcan_c(text: str) -> str:
    # prompt user for clock
    clock_rate = input("Enter the clock rate for fdcan: ")
    nbtq = input("Enter the nbtq for fdcan: ")

    # fix include
    include_pattern = r'^\s*#include\s+"fdcan\.h"\s*$'
    include_replacement = (
        '#include "fdcan.h"\n'
        '#include "pinmap.h"\n'
        '#include "peripheralmap.h"\n'
        '#include "stm32h7xx_hal.h"\n'
    )

    text = regex_replace_all(text,include_pattern,include_replacement)

    # comment out error handler
    text = regex_replace_all(
        text,
        r'Error_Handler\(\);',
        '//Error_Handler();'
    )

    # fix fdcan init signature
    text = regex_replace_all(
        text,
        r'void\s+(MX_.*_Init)\s*\(\s*void\s*\)',
        r'void \1(uint32_t baudrate)'
    )

    # replace .Init.NominalPrescaler = <any>;
    text = regex_replace_all(
        text,
        r'\.Init\.NominalPrescaler\s*=\s*\d+;',
        f".Init.NominalPrescaler = {clock_rate} / (baudrate * {nbtq});"
    )

    # replace fdcan_MspInit
    text = regex_replace_once(
        text,
        r'void\s+HAL_FDCAN_MspInit\s*\(\s*FDCAN_HandleTypeDef\s*\*\s*fdcanHandle\s*\)',
        'void HAL_FDCAN_MspInit_custom(FDCAN_GlobalTypeDef *fdcanHandle, Pin pin, uint8_t af)'
    )

    # replace fdcanHandle->Instance -> fdcanHandle
    text = regex_replace_all(
        text,
        r'fdcanHandle\s*->\s*Instance\s*==\s*([A-Za-z_]\w*\d+)',
        r'fdcanHandle == \1)'
    )

    # GPIO pin
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Pin\s*=\s*).*?;',
        r'\1pin.block_mask | pin.block_mask;'
    )

    # GPIO init
    text = regex_replace_all(
        text,
        r'HAL_GPIO_Init\s*\(\s*[A-Z0-9_]+,\s*&GPIO_InitStruct\s*\);',
        'HAL_GPIO_Init(pin.block, &GPIO_InitStruct);'
    )

    # af
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Alternate\s*=\s*).*?;',
        r'\1af;'
    )

    # remove HAL_FDCAN_MspDeInit
    text = remove_function_by_name(text, "HAL_FDCAN_MspDeInit")

    # insert dispatcher
    text = insert_fdcan_dispatcher(text)   

    return text

def fix_spi_c(text: str) -> str:
    # fix include
    include_pattern = r'^\s*#include\s+"spi\.h"\s*$'
    include_replacement = (
        '#include "pinmap.h"\n'
        '#include "peripheralmap.h"\n'
        '#include "stm32h7xx_hal.h"\n'
    )

    text = regex_replace_all(text,include_pattern,include_replacement)

    # comment out error handler
    text = regex_replace_all(
        text,
        r'Error_Handler\(\);',
        '//Error_Handler();'
    )

    # fix fdcan init signature
    text = regex_replace_all(
        text,
        r'void\s+(MX_.*_Init)\s*\(\s*void\s*\)',
        r'void \1(uint32_t baudrate_prescaler)'
    )

    # replace .Init.NominalPrescaler = <any>;
    text = regex_replace_all(
        text,
        r'\.Init\.BaudRatePrescaler\s*=\s*\d+;',
        r'.Init.BaudRatePrescaler = baudrate_prescaler;'
    )

    # replace fdcan_MspInit
    text = regex_replace_once(
        text,
        r'void\s+HAL_SPI_MspInit\s*\(\s*SPI_HandleTypeDef\s*\*\s*spiHandle\s*\)',
        'void HAL_SPI_MspInit_custom(SPI_TypeDef* spiHandle, Pin pin, uint8_t af)'
    )

    # replace spi->Instance -> spi
    text = regex_replace_all(
        text,
        r'spiHandle\s*->\s*Instance\s*==\s*([A-Za-z_]\w*\d+)',
        r'spiHandle == \1)'
    )

    # GPIO pin
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Pin\s*=\s*).*?;',
        r'\1pin.block_mask;'
    )

    # GPIO init
    text = regex_replace_all(
        text,
        r'HAL_GPIO_Init\s*\(\s*[A-Z0-9_]+,\s*&GPIO_InitStruct\s*\);',
        'HAL_GPIO_Init(pin.block, &GPIO_InitStruct);'
    )

    # af
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Alternate\s*=\s*).*?;',
        r'\1af;'
    )

    # remove HAL_FDCAN_MspDeInit
    text = remove_function_by_name(text, "HAL_SPI_MspDeInit")

    # insert dispatcher
    text = insert_spi_dispatcher(text)   

    return text

FIXERS = {
    "adc.c": fix_adc_c,
    "usart.c": fix_usart_c,
    "fdcan.c": fix_fdcan_c,
    "spi.c": fix_spi_c
}

# ---- main logic --------------------------------------------

def process_file(src: Path, dst: Path):
    text = src.read_text(encoding="utf-8")

    fixer = FIXERS.get(src.name)
    if fixer:
        text = fixer(text)

    dst.write_text(text, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("src_file", help="CubeIDE-generated source file")
    parser.add_argument("dst_file", help="Destination output file")
    args = parser.parse_args()

    src = Path(args.src_file)
    dst = Path(args.dst_file)

    if not src.exists():
        raise FileNotFoundError(f"Source file not found: {src}")

    dst.parent.mkdir(parents=True, exist_ok=True)

    process_file(src, dst)
    print(f"Imported {src.name} -> {dst}")


if __name__ == "__main__":
    main()
