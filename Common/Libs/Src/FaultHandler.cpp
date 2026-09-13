#include "FaultHandler.h"

#include <atomic>

#include "BPSCanStructs.h"
#include "MotorControllerCanStructs.h"
#include "Rivanna3SCanStructs.h"
#include "log.h"

namespace
{
    std::atomic<bool> bps_fault_active{false};
    std::atomic<bool> contactor_fault_active{false};
    std::atomic<bool> motor_controller_fault_active{false};
}

namespace FaultHandler
{
    void check_for_any_faults(const SerializedCanMessage &msg)
    {
        if (msg.id == BpsError::get_message_ID())
        {
            BpsError status{};
            status.deserialize(&msg);

            if (status.has_active_fault())
            {
                bps_fault_active.store(true);
                log_fault("BPS fault detected!");
            }
        }
        else if (msg.id == Contactor12Error::get_message_ID())
        {
            Contactor12Error status{};
            status.deserialize(&msg);

            if (status.has_active_fault())
            {
                contactor_fault_active = true;
                log_fault("Contactor fault detected!");
            }
        }
        else if (msg.id == MotorControllerError::get_message_ID())
        {
            MotorControllerError status{};
            status.deserialize(&msg);

            if (status.has_active_fault())
            {
                motor_controller_fault_active = true;
                log_fault("Motor controller fault detected!");
            }
        }
    }

    bool has_any_fault()
    {
        return bps_fault_active.load()
            || contactor_fault_active.load()
            || motor_controller_fault_active.load();
    }

    bool has_bps_fault()
    {
        return bps_fault_active.load();
    }

    bool has_contactor_fault()
    {
        return contactor_fault_active.load();
    }

    bool has_motor_controller_fault()
    {
        return motor_controller_fault_active.load();
    }
}