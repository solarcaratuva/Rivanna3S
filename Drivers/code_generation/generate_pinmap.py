import xml.etree.ElementTree as ET
import os
import re
import argparse

def parse_XML_pinmap(dir: str) -> dict:
    tree = ET.parse(dir)
    root = tree.getroot()

    pin_map = {}

    for pin in root.findall('{http://dummy.com}Pin'): # xmlns="http://dummy.com" attribute in XML's root tag makes all tags in XML have this "http://dummy.com" string before the tag name
        pin_name = pin.attrib['Name']
        position = int(pin.attrib['Position'])

        pattern = r"^P[A-Z](?:\d{1}|\d{2})$"

        if re.match(pattern, pin_name) and position <= 64:
            pin_map[pin_name] = position

    return pin_map

def create_header_file(file_path: str, pin_map: dict, family: str) -> None:
    with open(file_path, 'w') as f:
        # Header guard and includes
        f.write(f"""#ifndef PINMAP_H
#define PINMAP_H\n
#include "{family.lower()}xx.h"\n\n""")

        # Pin struct definition
        f.write("""typedef struct {
    GPIO_TypeDef* block;        // Pointer to GPIO peripheral block
    uint16_t block_mask;        // Mask for pins within the block
    uint64_t universal_mask;    // Unique global pin mask
} Pin;\n\n""")

        # Pins
        f.write("#define NC (Pin){NULL, 0, 0} // Not connected pin\n\n")
        for pin_name, pos in pin_map.items():
            block = pin_name[1]
            pin_num = pin_name[2:]
            mask_pos = pos - 1

            pin = pin_name[:2] + "_" + pin_num

            f.write(f"#define {pin} (Pin){{GPIO{block}, 1 << {pin_num}, 1ULL << {mask_pos}}}\n")
        

        # End header guard
        f.write(f"\n#endif /* PINMAP */")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate pinmap header file from XML",)
    parser.add_argument("--device_pinmapping", help="Path to the device pinmapping input XML file", required=True)
    parser.add_argument("--family", help="Name of the device family (e.g. STM32H743)", required=True)

    args = parser.parse_args()

    output_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pinmap.h')
    pin_map = parse_XML_pinmap(args.device_pinmapping)
    create_header_file(output_file_path, pin_map, args.family)

    
if __name__ == "__main__":
    main()