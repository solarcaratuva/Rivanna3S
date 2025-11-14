#include "blink.h"

void flashPin(DigitalOut pin) {
    if (pin.read() == true) { pin.write(false); }
    else { pin.write(true); }
}
