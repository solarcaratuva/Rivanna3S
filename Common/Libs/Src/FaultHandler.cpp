#include "FaultHandler.h"

#include <atomic>
#include <cstddef>

#include "BPSCanStructs.h"
#include "MotorControllerCanStructs.h"
#include "MPPTCanStructs.h"
#include "Rivanna3SCanStructs.h"
#include "log.h"

namespace
{
    std::atomic<bool> any_fault_active{false};
    std::atomic<bool> bps_fault_active{false};
    std::atomic<bool> contactor_fault_active{false};
    std::atomic<bool> motor_controller_fault_active{false};

    template <typename MessageType>
    void inspect_message(
        const SerializedCanMessage &msg,
        std::size_t expected_length,
        const char *message_name,
        std::atomic<bool> *specific_flag = nullptr)
    {
        if (expected_length > sizeof(msg.data) ||
            msg.len != expected_length)
        {
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

        log_fault("%s fault detected!", message_name);
    }
}

namespace FaultHandler
{
    void check_for_any_faults(const SerializedCanMessage &msg)
    {
        if (msg.id == AuxBatteryStatus::get_message_ID())
        {
            inspect_message<AuxBatteryStatus>(
                msg,
                RIVANNA3_S_AUX_BATTERY_STATUS_LENGTH,
                "AuxBatteryStatus");
        }
        else if (msg.id == MotorCommands::get_message_ID())
        {
            inspect_message<MotorCommands>(
                msg,
                RIVANNA3_S_MOTOR_COMMANDS_LENGTH,
                "MotorCommands");
        }
        else if (msg.id == DashboardCommands::get_message_ID())
        {
            inspect_message<DashboardCommands>(
                msg,
                RIVANNA3_S_DASHBOARD_COMMANDS_LENGTH,
                "DashboardCommands");
        }
        else if (msg.id == Heartbeat::get_message_ID())
        {
            inspect_message<Heartbeat>(
                msg,
                RIVANNA3_S_HEARTBEAT_LENGTH,
                "Heartbeat");
        }
        else if (msg.id == PedalStatus::get_message_ID())
        {
            inspect_message<PedalStatus>(
                msg,
                RIVANNA3_S_PEDAL_STATUS_LENGTH,
                "PedalStatus");
        }
        else if (msg.id == PrechargeStatus::get_message_ID())
        {
            inspect_message<PrechargeStatus>(
                msg,
                RIVANNA3_S_PRECHARGE_STATUS_LENGTH,
                "PrechargeStatus");
        }
        else if (msg.id == Contactor12Error::get_message_ID())
        {
            inspect_message<Contactor12Error>(
                msg,
                RIVANNA3_S_CONTACTOR12_ERROR_LENGTH,
                "Contactor12Error",
                &contactor_fault_active);
        }
        else if (msg.id == UpdateControl::get_message_ID())
        {
            inspect_message<UpdateControl>(
                msg,
                RIVANNA3_S_UPDATE_CONTROL_LENGTH,
                "UpdateControl");
        }
        else if (msg.id == UpdateData::get_message_ID())
        {
            inspect_message<UpdateData>(
                msg,
                RIVANNA3_S_UPDATE_DATA_LENGTH,
                "UpdateData");
        }
        else if (msg.id == BpsStatus::get_message_ID())
        {
            inspect_message<BpsStatus>(
                msg,
                BPS_BPS_STATUS_LENGTH,
                "BpsStatus");
        }
        else if (msg.id == BpsError::get_message_ID())
        {
            inspect_message<BpsError>(
                msg,
                BPS_BPS_ERROR_LENGTH,
                "BpsError",
                &bps_fault_active);
        }
        else if (msg.id == MG0Status::get_message_ID())
        {
            inspect_message<MG0Status>(
                msg,
                MPPT_MG0_STATUS_LENGTH,
                "MG0Status");
        }
        else if (msg.id == MG1Status::get_message_ID())
        {
            inspect_message<MG1Status>(
                msg,
                MPPT_MG1_STATUS_LENGTH,
                "MG1Status");
        }
        else if (msg.id == MG0OutputVoltageInputPower::get_message_ID())
        {
            inspect_message<MG0OutputVoltageInputPower>(
                msg,
                MPPT_MG0_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH,
                "MG0OutputVoltageInputPower");
        }
        else if (msg.id == MG1OutputVoltageInputPower::get_message_ID())
        {
            inspect_message<MG1OutputVoltageInputPower>(
                msg,
                MPPT_MG1_OUTPUT_VOLTAGE_INPUT_POWER_LENGTH,
                "MG1OutputVoltageInputPower");
        }
        else if (msg.id == MG0PCBMOSFETTemperature::get_message_ID())
        {
            inspect_message<MG0PCBMOSFETTemperature>(
                msg,
                MPPT_MG0_PCBMOSFET_TEMPERATURE_LENGTH,
                "MG0PCBMOSFETTemperature");
        }
        else if (msg.id == MG1PCBMOSFETTemperature::get_message_ID())
        {
            inspect_message<MG1PCBMOSFETTemperature>(
                msg,
                MPPT_MG1_PCBMOSFET_TEMPERATURE_LENGTH,
                "MG1PCBMOSFETTemperature");
        }
        else if (msg.id == MotorControllerFrameRequest::get_message_ID())
        {
            inspect_message<MotorControllerFrameRequest>(
                msg,
                MOTOR_CONTROLLER_MOTOR_CONTROLLER_FRAME_REQUEST_LENGTH,
                "MotorControllerFrameRequest");
        }
        else if (msg.id == MotorControllerPowerStatus::get_message_ID())
        {
            inspect_message<MotorControllerPowerStatus>(
                msg,
                MOTOR_CONTROLLER_MOTOR_CONTROLLER_POWER_STATUS_LENGTH,
                "MotorControllerPowerStatus");
        }
        else if (msg.id == MotorControllerDriveStatus::get_message_ID())
        {
            inspect_message<MotorControllerDriveStatus>(
                msg,
                MOTOR_CONTROLLER_MOTOR_CONTROLLER_DRIVE_STATUS_LENGTH,
                "MotorControllerDriveStatus");
        }
        else if (msg.id == MotorControllerError::get_message_ID())
        {
            inspect_message<MotorControllerError>(
                msg,
                MOTOR_CONTROLLER_MOTOR_CONTROLLER_ERROR_LENGTH,
                "MotorControllerError",
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
