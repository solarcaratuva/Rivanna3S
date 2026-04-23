#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <cstdint>

#include "Timeout.h"
#include "CanInterface.h"
#include "nodes.h"
#include "FiniteQueue.h"

using Callback = std::function<void()>;

/**
 * Always disabled boards — do NOT edit this list.
 * These boards are never monitored regardless of DISABLED_BOARDS.
 */
constexpr Node ALWAYS_DISABLED_BOARDS[] = {
    Node::MotorController,
    Node::BMS
};
constexpr int ALWAYS_DISABLED_BOARDS_COUNT =
    sizeof(ALWAYS_DISABLED_BOARDS) / sizeof(ALWAYS_DISABLED_BOARDS[0]);

/**
 * Edit this list to disable specific boards from heartbeat monitoring.
 */
constexpr Node DISABLED_BOARDS[] = {
    // Node::BottomDistBoard,
    // Node::MotorBoard,
    // Node::RelayBoard,
    // Node::TelemetryBoard,
    // Node::TopDistBoard,
    // Node::WheelBoard,
};
constexpr int DISABLED_BOARDS_COUNT =
    sizeof(DISABLED_BOARDS) / sizeof(DISABLED_BOARDS[0]);

/**
 * Heartbeat safety monitor system.
 * Uses CAN callbacks via CanInterface (no polling).
 */
class HeartbeatSafetySystem {
public:
    static constexpr uint32_t HEARTBEAT_SEND_INTERVAL_MS = 100;
    static constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 500;

    /**
    * @brief Initializes the HeartbeatSafetySystem.
    *
    * Configures the CAN interface, registers the heartbeat receive callback,
    * initializes timeout handlers for all enabled nodes, and starts the
    * background sender thread responsible for periodically transmitting
    * this board's heartbeat message.
    *
    * Disabled boards are skipped during timeout setup based on configuration:
    * * Permanently disabled boards are ignored silently.
    * * Conditionally disabled boards are skipped with a log message.
    *
    * @param can_interface Pointer to the CAN interface used for sending and receiving heartbeat messages.
    * @param missed_heartbeat_callback Callback function invoked when a heartbeat timeout is detected.
    * @param this_board Identifier of the current node/board.
    */
    static void setup(CanInterface* can_interface,
                      Callback missed_heartbeat_callback,
                      Node self_board);

private:
    static void heartbeat_can_callback(const SerializedCanMessage &msg);
    static void sender_task();
    static void sender_queue_task();
    static void timeout_triggered(uint8_t board_idx);

    static int is_board_disabled(Node board);

    static CanInterface* can;
    static Callback missed_cb;
    static Node self_board;
    static FiniteQueue callback_queue;

    static Timeout timeouts[NUM_NODES];
};

#endif // HEARTBEAT_H