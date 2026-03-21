import argparse
import re
from pathlib import Path

CHANGE_COMMENT = " // <-- This line changed when importing"

<<<<<<< HEAD
I2C_HASHES: dict[int, str] = {}
=======
MIN_HEAP_SIZE = "0x1FFF"
MIN_STACK_SIZE = "0x1FFF"

I2C_REGS: dict[int, str] = {}
>>>>>>> main
I2C_HELPER_TEXT: str = ""

# i2c helpers
def extract_i2c_timing(text: str) -> str | None:
    m = re.search(
        r'\.Init\.Timing\s*=\s*(0x[0-9A-Fa-f]+|\d+);',
        text
    )
    return m.group(1) if m else None

<<<<<<< HEAD
def collect_i2c_hashes(src_dir: Path):
    mapping = {
        "i2c1.c": 100000,
        "i2c2.c": 400000,
        "i2c.c": 1000000,
=======
def collect_i2c_REGS(src_dir: Path):
    mapping = {
        "i2c1.c": 100000,
        "i2c2.c": 400000,
        "i2c3.c": 1000000,
>>>>>>> main
    }

    for name, baud in mapping.items():
        path = src_dir / name
        if not path.exists():
<<<<<<< HEAD
            print(f'ERROR: {name} file not found, set timing register hash for the baudrate associated with this file to 0')
            I2C_HASHES[baud] = 0
            continue

        text = path.read_text(encoding="utf-8")
        timing = extract_i2c_timing(text)

        if timing:
            I2C_HASHES[baud] = timing
        else:
            print(f'ERROR: timing in {name} file not found, set timing register hash for the baudrate associated with this file to 0')
            I2C_HASHES[baud] = 0

def generate_i2c_helper() -> str:
    if not I2C_HASHES:
        return ""

    lines = [
        "static uint32_t baudrate_to_hash(uint32_t baudrate)",
=======
            print(f'ERROR: {name} file not found, set timing register for the baudrate associated with this file to 0')
            I2C_REGS[baud] = 0
            continue

        text = path.read_text(encoding="latin1")
        timing = extract_i2c_timing(text)

        if timing:
            I2C_REGS[baud] = timing
        else:
            print(f'ERROR: timing in {name} file not found, set timing register for the baudrate associated with this file to 0')
            I2C_REGS[baud] = 0

def generate_i2c_helper() -> str:
    if not I2C_REGS:
        return ""

    lines = [
        "uint32_t compute_timing(uint32_t baudrate)",
>>>>>>> main
        "{",
        "    switch (baudrate)",
        "    {",
    ]

<<<<<<< HEAD
    for baud in sorted(I2C_HASHES):
        lines.append(f"    case {baud}: return {I2C_HASHES[baud]};")
=======
    for baud in sorted(I2C_REGS):
        lines.append(f"    case {baud}: return {I2C_REGS[baud]};")
>>>>>>> main

    lines += [
        "    default: return 0;",
        "    }",
        "}",
        "",
    ]

    return "\n".join(lines)

<<<<<<< HEAD
=======

>>>>>>> main
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
        rf'\n*\s*void\s+{func_name}\s*\([^)]*\)\s*\{{',
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

def insert_dispatcher(
    text: str,
    *,
    init_regex: str,
    dispatcher_name: str,
    handle_type: str,
    instance_type: str,
    instance_prefixes: tuple,
    handle_prefix: str,
    init_params: str,
    init_call_args: str,
) -> str:
    matches = re.findall(init_regex, text)
    if not matches:
        return text

    # Normalize match list
    inits = []
    seen = set()
    for m in matches:
        init = m if isinstance(m, str) else m[0]
        if init not in seen:
            seen.add(init)
            inits.append(init)

    func = (
        f"\n\n{handle_type}* {dispatcher_name}({instance_type}* inst"
        f"{', ' + init_params if init_params else ''}) {{\n"
    )

    for idx, init in enumerate(inits):
        base = init.replace("MX_", "").replace("_Init", "")
        num_match = re.search(r'(\d+)(?!.*\d)', base)
        if not num_match:
            continue

        n = num_match.group(1)

        # Determine peripheral instance
        matched_prefix = None
        instance = None
        for p in instance_prefixes:
            if base.startswith(p):
                instance = f"{p}{n}"
                matched_prefix = p
                break

        if instance is None:
            continue

        if matched_prefix == "USART":
            handle = f"huart{n}"
        elif matched_prefix == "LPUART":
            handle = f"hlpuart{n}"
        else:
            handle = f"{handle_prefix}{matched_prefix.lower()}{n}"

        if idx == 0:
            func += f"    if (inst == {instance}) {{\n"
        else:
            func += f"    else if (inst == {instance}) {{\n"

        call_args = init_call_args if init_call_args else ""
        func += (
            f"        {init}({call_args});\n"
            f"        return &{handle};\n"
            "    }\n"
        )

    func += (
        "    else {\n"
        "        return NULL;\n"
        "    }\n"
        "}\n"
    )

    return text.rstrip() + func

# per-file fixes

def fix_adc_c(text: str) -> str:
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
    replacement = r'  sConfig.Channel = channel;'

    text = regex_replace_all(text, pattern, replacement)

    # fix ranks
    pattern = r'^\s*sConfig\.Rank\s*=.*?;'
    replacement = r'  sConfig.Rank = rank;'

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

    # remove deinit
    text = remove_function_by_name(text, "HAL_ADC_MspDeInit")

    # insert adc_dispatcher
    text = insert_dispatcher(
        text,
        init_regex=r'\bMX_ADC\d+_Init\s*',
        dispatcher_name="ADC_init",
        handle_type="ADC_HandleTypeDef",
        instance_type="ADC_TypeDef",
        instance_prefixes=("ADC",),
        handle_prefix="h",
        init_params="uint32_t channel, uint32_t rank",
        init_call_args="channel, rank",
    )

    return text

def fix_usart_c(text: str) -> str:
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

    # remove HAL_UART_MspDeInit
    text = remove_function_by_name(text, "HAL_UART_MspDeInit")

    # insert dispatcher
    text = insert_dispatcher(
        text,
        init_regex=r'\b(MX_(?:UART\d+|USART\d+_UART|LPUART\d+_UART)_Init)\s*\(',
        dispatcher_name="UART_init",
        handle_type="UART_HandleTypeDef",
        instance_type="USART_TypeDef",
        instance_prefixes=("UART", "USART", "LPUART"),
        handle_prefix="h",
        init_params="uint32_t baudrate",
        init_call_args="baudrate",
    )

    return text

def fix_fdcan_c(text: str) -> str:
    # prompt user for clock
    clock_rate = int(input("Enter the clock rate for fdcan (in MHz): ")) * 1000000 # converts MHz to Hz

    # fix fdcan init signature
    text = regex_replace_all(
        text,
        r'void\s+(MX_.*_Init)\s*\(\s*void\s*\)',
        r'void \1(uint32_t baudrate)'
    )

    # replace .Init.NominalPrescaler = <any>;
    text = regex_replace_all(
        text,
        r'(hfdcan\d+)\.Init\.NominalPrescaler\s*=\s*\d+;',
        rf"\g<1>.Init.NominalPrescaler = calculate_prescaler(\g<1>, {clock_rate}, baudrate);"
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

    # remove HAL_FDCAN_MspDeInit
    text = remove_function_by_name(text, "HAL_FDCAN_MspDeInit")

    # insert dispatcher
    text = insert_dispatcher(
        text,
        init_regex=r'\b(MX_FDCAN\d+_Init)\s*\(',
        dispatcher_name="FDCAN_init",
        handle_type="FDCAN_HandleTypeDef",
        instance_type="FDCAN_GlobalTypeDef",
        instance_prefixes=("FDCAN",),
        handle_prefix="h",
        init_params="uint32_t baudrate",
        init_call_args="baudrate",
    )

    # insert calculate prescaler function
    func = """
uint32_t calculate_prescaler(FDCAN_HandleTypeDef *hfdcan, uint32_t peripheral_clock, uint32_t baudrate) {
    uint32_t time_quanta = 1 + hfdcan->Init.NominalTimeSeg1 + hfdcan->Init.NominalTimeSeg2;
    return peripheral_clock / (baudrate * time_quanta);
}
    """
    decl = "uint32_t calculate_prescaler(FDCAN_HandleTypeDef *hfdcan, uint32_t peripheral_clock, uint32_t baudrate);\n"
    text = decl + "\n" + text.rstrip() + "\n\n" + func

    return text

def fix_spi_c(text: str) -> str:
    # prompt user for clock
    baudrate = input("Enter the baud rate for spi when prescaler is 2: ")
    clock_rate = int(baudrate) * 2
    # fix fdcan init signature
    text = regex_replace_all(
        text,
        r'void\s+(MX_.*_Init)\s*\(\s*void\s*\)',
        r'void \1(uint32_t baudrate_prescaler)'
    )

    # make datasize 8 bits
    text = regex_replace_all(
        text,
        r'\.Init\.DataSize\s*=\s*SPI_DATASIZE_\d+BIT;',
        rf'.Init.DataSize = SPI_DATASIZE_8BIT;'
    )

    # replace .Init.NominalPrescaler = <any>;
    text = regex_replace_all(
        text,
        r'\.Init\.BaudRatePrescaler\s*=\s*SPI_BAUDRATEPRESCALER_\d+;',
        rf'.Init.BaudRatePrescaler = spi_prescaler_from_baud({clock_rate}, baudrate_prescaler);'
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

    # remove HAL_FDCAN_MspDeInit
    text = remove_function_by_name(text, "HAL_SPI_MspDeInit")

    # insert dispatcher
    text = insert_dispatcher(
        text,
        init_regex=r'\b(MX_SPI\d+_Init)\s*\(',
        dispatcher_name="SPI_init",
        handle_type="SPI_HandleTypeDef",
        instance_type="SPI_TypeDef",
        instance_prefixes=("SPI",),
        handle_prefix="h",
        init_params = "uint32_t baudrate_prescaler",
        init_call_args="baudrate_prescaler",
    )

    func = """
static uint32_t spi_prescaler_from_baud(uint32_t periph_clk, uint32_t target_baud)
{
    uint32_t div = (periph_clk + target_baud - 1) / target_baud;

    if (div <= 2)   return SPI_BAUDRATEPRESCALER_2;
    if (div <= 4)   return SPI_BAUDRATEPRESCALER_4;
    if (div <= 8)   return SPI_BAUDRATEPRESCALER_8;
    if (div <= 16)  return SPI_BAUDRATEPRESCALER_16;
    if (div <= 32)  return SPI_BAUDRATEPRESCALER_32;
    if (div <= 64)  return SPI_BAUDRATEPRESCALER_64;
    if (div <= 128) return SPI_BAUDRATEPRESCALER_128;

    return SPI_BAUDRATEPRESCALER_256;
}
    """
    decl = "static uint32_t spi_prescaler_from_baud(uint32_t periph_clk, uint32_t target_baud);\n"
    text = decl + "\n" + text.rstrip() + "\n\n" + func

    return text

def fix_i2c_c(text: str) -> str:
    # fix i2c init signature
    text = regex_replace_all(
        text,
        r'void\s+(MX_.*_Init)\s*\(\s*void\s*\)',
        r'void \1(uint32_t baudrate)'
    )

    # replace timing assignment
    text = regex_replace_all(
        text,
        r'\.Init\.Timing\s*=\s*(?:\d+|0x[0-9A-Fa-f]+);',
<<<<<<< HEAD
        r'.Init.Timing = baudrate_to_hash(baudrate);'
=======
        r'.Init.Timing = compute_timing(baudrate);'
>>>>>>> main
    )

    # replace fdcan_MspInit
    text = regex_replace_once(
        text,
        r'void\s+HAL_I2C_MspInit\s*\(\s*I2C_HandleTypeDef\s*\*\s*i2cHandle\s*\)',
        'void HAL_I2C_MspInit_custom(I2C_TypeDef* i2cHandle, Pin pin, uint8_t af)'
    )

    # replace i2cHandle->Instance
    text = regex_replace_all(
        text,
        r'i2cHandle\s*->\s*Instance\s*==\s*([A-Za-z_]\w*\d+)',
        r'i2cHandle == \1)'
    )

    # GPIO pin
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Pin\s*=\s*).*?;',
        r'\1pin.block_mask;'
    )

    # remove HAL_i2c_MspDeInit
    text = remove_function_by_name(text, "HAL_I2C_MspDeInit")

    # insert dispatcher
    text = insert_dispatcher(
        text,
        init_regex=r'\b(MX_I2C\d+_Init)\s*\(',
        dispatcher_name="I2C_init",
        handle_type="I2C_HandleTypeDef",
        instance_type="I2C_TypeDef",
        instance_prefixes=("I2C",),
        handle_prefix="h",
        init_params="uint32_t baudrate",
        init_call_args="baudrate",
    )

<<<<<<< HEAD
    # add helper at top of i2c.c
    text = I2C_HELPER_TEXT + "\n" + text

=======
>>>>>>> main
    return text

FIXERS = {
    "adc.c": fix_adc_c,
    "usart.c": fix_usart_c,
    "fdcan.c": fix_fdcan_c,
    "spi.c": fix_spi_c,
<<<<<<< HEAD
    "i2c.c": fix_i2c_c
=======
    "i2c3.c": fix_i2c_c
>>>>>>> main
}

# ---- main logic --------------------------------------------

def process_file(src: Path, dst: Path):
<<<<<<< HEAD
    text = src.read_text(encoding="utf-8")

    # fix includes
    include_pattern = r'^\s*#include\s+"[^"]+\.h"\s*$'
=======
    text = src.read_text(encoding="latin1")

    # fix includes
    include_pattern = r'^\s*#include\s+"[^"]+\.h"\s*$'

>>>>>>> main
    include_replacement = (
        '#include "pinmap.h"\n'
        '#include "peripheralmap.h"\n'
        '#include "stm32h7xx_hal.h"\n'
    )

<<<<<<< HEAD
=======
    if (src.name == "i2c3.c"):
        include_replacement = include_replacement + I2C_HELPER_TEXT

>>>>>>> main
    text = regex_replace_all(text,include_pattern,include_replacement)

    # fix alternate function
    text = regex_replace_all(
        text,
        r'^(\s*GPIO_InitStruct\.Alternate\s*=\s*).*?;',
        r'\1af;'
    )

    # fix GPIO init
    text = regex_replace_all(
        text,
        r'HAL_GPIO_Init\s*\(\s*[A-Z0-9_]+,\s*&GPIO_InitStruct\s*\);',
        'HAL_GPIO_Init(pin.block, &GPIO_InitStruct);'
    )

    # comment out error handler
    text = regex_replace_all(
        text,
        r'Error_Handler\(\);',
        '//Error_Handler();'
    )

    fixer = FIXERS.get(src.name)
    if fixer:
        text = fixer(text)
<<<<<<< HEAD
        dst.write_text(text, encoding="utf-8")
=======
        dst.write_text(text)
>>>>>>> main
        return 0
    
    return 1

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("src_dir", help="directory with CubeIDE-generated source files")
    parser.add_argument("dst_dir", help="directory for destination output files")
    args = parser.parse_args()

    src = Path(args.src_dir)
    dst = Path(args.dst_dir)

    if not src.exists():
        raise FileNotFoundError(f"Source directory not found: {src}")

    dst.mkdir(parents=True, exist_ok=True)

<<<<<<< HEAD
    print(
        "For i2c, make sure you have 3 variants of i2c, with baudrates set to "
        "100000, 400000, and 1000000.\n"
        "Name them i2c1.c, i2c2.c, and i2c.c respectively.\n"
        "Only i2c.c will be imported.\n"
        "Note that only the first i2c peripheral in each file need to be set to the baudrate!\n"
    )

    collect_i2c_hashes(src)
=======
    # print(
    #     "For i2c, make sure you have 3 variants of i2c, with baudrates set to "
    #     "100000, 400000, and 1000000.\n"
    #     "Name them i2c1.c, i2c2.c, and i2c3.c respectively.\n"
    #     "Only peripherals from i2c3.c will be imported into i2c.c\n"
    #     "Note that only the first i2c peripheral in each file need to be set to the baudrate!\n"
    # )

    collect_i2c_REGS(src)
>>>>>>> main

    global I2C_HELPER_TEXT
    I2C_HELPER_TEXT = generate_i2c_helper()

    for file in src.iterdir():
        if file.is_file():
<<<<<<< HEAD
            out = dst / file.name
=======
            if (file.name == "i2c3.c"):
                out = dst / "i2c.c"

            elif (file.name.endswith("_FLASH.ld")):
                out = dst / file.name
                text = file.read_text(encoding="latin1")

                text = re.sub(r"_Min_Heap_Size\s*=\s*0x[0-9A-Fa-f]+;", "_Min_Heap_Size = {MIN_HEAP_SIZE}}; // <- this line changed", text)
                text = re.sub(r"_Min_Stack_Size\s*=\s*0x[0-9A-Fa-f]+;", "_Min_Stack_Size = {MIN_STACK_SIZE}; // <- this line changed", text)

                out.write_text(text)
                continue
            else:
                out = dst / file.name

>>>>>>> main
            if (not process_file(file, out)):
                print(f"Imported {file.name} -> {out}")

if __name__ == "__main__":
<<<<<<< HEAD
    main()
=======
    main()
>>>>>>> main
