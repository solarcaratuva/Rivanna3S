#ifndef FAULT_HANDLER_H
#define FAULT_HANDLER_H

#include "can.h"

namespace FaultHandler
{
    void check_for_any_faults(const SerializedCanMessage &msg);

    bool has_any_fault();
    bool has_bps_fault();
    bool has_contactor_fault();
    bool has_motor_controller_fault();
}

#endif