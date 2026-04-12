#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <cstdint>

#include "Timeout.h"
#include "CanInterface.h"
#include "nodes.h"

using Callback = std::function<void()>;

/**
 * Always disabled boards — should NOT edit this list.
 * These boards are never monitored regardless of DISABLED_BOARDS.
 */
const Node HeartbeatSafetySystem::ALWAYS_DISABLED_BOARDS[] = {
    Node::MotorController,
    Node::BMS
};
const int HeartbeatSafetySystem::ALWAYS_DISABLED_BOARDS_COUNT =
    sizeof(ALWAYS_DISABLED_BOARDS) / sizeof(ALWAYS_DISABLED_BOARDS[0]);

/**
 * Edit this list to disable specific boards from heartbeat monitoring.
 */
const Node HeartbeatSafetySystem::DISABLED_BOARDS[] = {
    // e.g. Node::TelemetryBoard
};
const int HeartbeatSafetySystem::DISABLED_BOARDS_COUNT =
    sizeof(DISABLED_BOARDS) / sizeof(DISABLED_BOARDS[0]);

/**
 * Heartbeat safety monitor system.
 * Uses CAN callbacks via CanInterface (no polling).
 */
class HeartbeatSafetySystem {
public:
    static constexpr uint32_t HEARTBEAT_SEND_INTERVAL_MS = 100;
    static constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 250;

    /**
     * Boards that are permanently excluded from heartbeat monitoring.
     * Do NOT edit this list.
     */
    static const Node ALWAYS_DISABLED_BOARDS[];
    static const int  ALWAYS_DISABLED_BOARDS_COUNT;

    /**
     * Boards that are currently disabled from heartbeat monitoring.
     * Edit this list to enable or disable boards.
     */
    static const Node DISABLED_BOARDS[];
    static const int  DISABLED_BOARDS_COUNT;

    static void setup(CanInterface* can_interface,
                      Callback missed_heartbeat_callback,
                      Node self_board);

private:
    static void heartbeat_can_callback(const SerializedCanMessage &msg);
    static void sender_task();
    static void timeout_triggered(int board_idx);

    static bool is_board_enabled(Node board);

    static CanInterface* can;
    static Callback missed_cb;
    static Node self;

    static Timeout timeouts[NUM_NODES];
};

#endif // HEARTBEAT_H