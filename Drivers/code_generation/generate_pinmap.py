import xml.etree.ElementTree as ET
import os
import re

def parse_XML_pinmap(dir):
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

def create_header_file(file_path, pin_map):
    with open(file_path, 'w') as f:
        # Header guard and includes
        f.write(f"#ifndef PINMAP_H\n")
        f.write(f"#define PINMAP_H\n\n")
        f.write("#include \"stm32h743xx.h\"\n\n")

        # Pin struct definition
        f.write("typedef struct {\n")
        f.write("    GPIO_TypeDef* block;       // Pointer to GPIO peripheral block\n")
        f.write("    uint16_t block_mask;       // Mask for pins within the block\n")
        f.write("    uint64_t universal_mask;   // Unique global pin mask\n")
        f.write("} Pin;\n\n")

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


if __name__ == "__main__":
    dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'input/STM32H743ZITx.xml')
    pin_map = parse_XML_pinmap(dir)

    file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pinmap.h')
    create_header_file(file_path, pin_map)




    
