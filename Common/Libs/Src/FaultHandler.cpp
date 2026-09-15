#include "FaultHandler.h"

#include <atomic>
#include <cstddef>

#include "BPSCanStructs.h"
#include "MotorControllerCanStructs.h"
#include "Rivanna3SCanStructs.h"
#include "log.h"

namespace
{
    std::atomic<bool> any_fault_active{false};
    std::atomic<bool> bps_fault_active{false};
    std::atomic<bool> contactor_fault_active{false};
    std::atomic<bool> motor_controller_fault_active{false};

    template <typename MessageType>
    void record_fault_if_present(
        const SerializedCanMessage &msg,
        std::size_t expected_length,
        std::atomic<bool> *specific_flag = nullptr)
    {
        if (msg.len != expected_length)
        {
            log_warn("Unexpected length for CAN ID 0x%X: got %u, expected %zu",
                     msg.id, msg.len, expected_length);
            return;
        }

        MessageType status{};
        status.deserialize(&msg);

        if (!status.has_active_fault())
        {
            return;
        }

        any_fault_active.store(true);

        if (specific_flag != nullptr)
        {
            specific_flag->store(true);
        }
        status.log_msg(FAULT_LVL);
    }
}

namespace FaultHandler
{
    void check_for_any_faults(const SerializedCanMessage &msg)
    {
        // Only these three message types carry fault signals (has_active_fault()
        // is a hardcoded `return 0;` stub on every other generated CAN struct).
        if (msg.id == Contactor12Error::get_message_ID())
        {
            record_fault_if_present<Contactor12Error>(
                msg,
                RIVANNA3_S_CONTACTOR12_ERROR_LENGTH,
                &contactor_fault_active);
        }
        else if (msg.id == BpsError::get_message_ID())
        {
            record_fault_if_present<BpsError>(
                msg,
                BPS_BPS_ERROR_LENGTH,
                &bps_fault_active);
        }
        else if (msg.id == MotorControllerError::get_message_ID())
        {
            record_fault_if_present<MotorControllerError>(
                msg,
                MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_LENGTH,
                &motor_controller_fault_active);
        }
    }

    bool has_any_fault()
    {
        return any_fault_active.load();
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
