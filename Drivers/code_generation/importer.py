'''
parameter for directory generated peripherals where it is
parameter where to put imported files

fix adc.c

Fix includes
replace:
#include "adc.h"
with:
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"


'''

