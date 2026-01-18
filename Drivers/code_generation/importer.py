#!/usr/bin/env python3

import argparse
import re
from pathlib import Path

# ---- regex helpers ----------------------------------------

def regex_replace_once(text: str, pattern: str, replacement: str) -> str:
    return re.sub(pattern, replacement, text, count=1, flags=re.MULTILINE)

def regex_replace_all(text: str, pattern: str, replacement: str) -> str:
    return re.sub(pattern, replacement, text, count=0, flags=re.MULTILINE)

# ---- per-file fixes ---------------------------------------

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
    initname_pattern = r'Error_Handler();'
    initname_replacement = r'//Error_Handler();'

    text = regex_replace_all(initname_pattern,initname_replacement)

    # fix init names
    pattern = r'void\s+MX_ADC\d+_Init\s*\(\s*void\s*\)'
    replacement = r'void MX_ADC\1_Init(uint32_t channel, uint32_t rank)'

    text = regex_replace_all(pattern,replacement)

    # fix channels
    pattern = r'^\s*sConfig\.Channel\s*=.*?;'
    replacement = r'sConfig.Channel = channel;'

    text = regex_replace_all(pattern,replacement)

    # fix ranks
    pattern = r'^\s*sConfig\.Rank\s*=.*?;'
    replacement = r'sConfig.Rank = rank;'

    text = regex_replace_all(pattern,replacement)

    # generate the ADC_init
    pattern = r'MX_ADC(\d+)_Init'
    matches = re.findall(pattern, text)

    adc_list= sorted(set(int(m) for m in matches))

    # code from AI
    lines = []
    lines.append("ADC_HandleTypeDef* ADC_init(const ADC_TypeDef* hadc, uint32_t channel, uint32_t rank) {")
    
    for adc_num in adc_list:
        if adc_num == adc_list[0]:
            lines.append(f"  if (hadc == ADC{adc_num}) {{")
        else:
            lines.append(f"  else if (hadc == ADC{adc_num}) {{")
        
        lines.append(f"    MX_ADC{adc_num}_Init(channel, rank);")
        lines.append(f"    return &hadc{adc_num};")
        lines.append("  }")
    
    # Default return
    lines.append(f"  return &hadc{adc_list[0]}; // Default return to avoid compiler warning")
    lines.append("}")
    
    return "\n".join(lines)

    with open("adc.c", "r") as f:
        text = f.read()

    adc_list = find_adc_instances(text)
    adc_init_code = generate_adc_init_function(adc_list)

    print(adc_init_code)



    return text


FIXERS = {
    "adc.c": fix_adc_c,
    # "spi.c": fix_spi_c,
    # "i2c.c": fix_i2c_c,
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
    parser.add_argument("src_dir", help="CubeIDE-generated peripheral directory")
    parser.add_argument("dst_dir", help="Destination directory")
    args = parser.parse_args()

    src_dir = Path(args.src_dir)
    dst_dir = Path(args.dst_dir)
    dst_dir.mkdir(parents=True, exist_ok=True)

    for src in src_dir.glob("*.c"):
        dst = dst_dir / src.name
        process_file(src, dst)
        print(f"Imported {src.name}")


if __name__ == "__main__":
    main()
