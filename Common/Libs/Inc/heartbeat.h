#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <cstdint>
#include <functional>

#include "Timeout.h"
#include "CanInterface.h"

using Callback = std::function<void()>;

/**
 * Heartbeat safety monitor system.
 * Uses CAN callbacks via CanInterface (no polling).
 */
class HeartbeatSafetySystem {
public:
    static constexpr uint32_t HEARTBEAT_SEND_INTERVAL_MS = 100;
    static constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 250;

    // Enable / disable boards
    static constexpr bool BOARD_MOTOR_ENABLE      = true;
    static constexpr bool BOARD_RELAY_ENABLE      = true;
    static constexpr bool BOARD_TELEMETRY_ENABLE  = true;
    static constexpr bool BOARD_TOPDIST_ENABLE    = true;
    static constexpr bool BOARD_BOTDIST_ENABLE    = true;

    enum class Board : uint8_t {
        MOTOR = 0,
        RELAY,
        TELEMETRY,
        TOPDIST,
        BOTDIST,
        COUNT
    };

    static void setup(CanInterface* can_interface,
                      Callback missed_heartbeat_callback,
                      Board self_board);

private:
    static void heartbeat_can_callback(const SerializedCanMessage &msg);
    static void sender_task();
    static void handle_received_heartbeat(uint16_t board_id);
    static void timeout_triggered(uint16_t board_id);

    static CanInterface* can;
    static Callback missed_cb;
    static Board self;

    static Timeout timeouts[static_cast<int>(Board::COUNT)];
    static bool board_enabled[static_cast<int>(Board::COUNT)];
};

#endif // HEARTBEAT_H