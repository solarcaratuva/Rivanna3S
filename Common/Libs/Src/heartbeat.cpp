#include "heartbeat.h"
#include "Rivanna3SCanStructs.h"
#include "thread.h"
#include "clock.h"
#include "log.h"

// --- Static member definitions ---

CanInterface*  HeartbeatSafetySystem::can       = nullptr;
Callback       HeartbeatSafetySystem::missed_cb = nullptr;
Node           HeartbeatSafetySystem::self_board;
Timeout        HeartbeatSafetySystem::timeouts[NUM_NODES];

// --- CAN callback (also handles the received heartbeat) ---

void HeartbeatSafetySystem::heartbeat_can_callback(const SerializedCanMessage &msg)
{
    Heartbeat hb{};
    hb.deserialize(&msg);

    Node sender = static_cast<Node>(hb.source);
    int  idx    = static_cast<int>(sender);

    if (is_board_disabled(sender))   
        return;

    timeouts[idx].refresh();
}

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

void HeartbeatSafetySystem::setup(CanInterface* can_interface,
                                  Callback missed_heartbeat_callback,
                                  Node this_board)
{
    can       = can_interface;
    missed_cb = missed_heartbeat_callback;
    self_board      = this_board;

    can->register_callback(Heartbeat::get_message_ID(),
                           HeartbeatSafetySystem::heartbeat_can_callback);

    for (int i = 0; i < NUM_NODES; i++) {
        Node board = static_cast<Node>(i);

        if (is_board_disabled(board) == 1) {
            log_info("Heartbeat: Board %d disabled (this board is not an always-disabled board)", i); // only log the boards that are not permanently disabled
            continue;
        }
        else if (is_board_disabled(board) == 2) {
            continue;
        }

        timeouts[i].attach([i]() {
            timeout_triggered(i);
        }, HEARTBEAT_TIMEOUT_MS);
    }

    static Thread sender_thread;
    sender_thread.start(sender_task);
}

// --- Sender ---

void HeartbeatSafetySystem::sender_task()
{
    Clock clock;

    while (true) {
        Heartbeat hb{};
        hb.source = static_cast<uint8_t>(self_board);

        SerializedCanMessage scm{};
        hb.serialize(&scm);

        can->write(&scm);

        clock.sleep_for(HEARTBEAT_SEND_INTERVAL_MS);
    }
}

// --- Timeout handler ---

void HeartbeatSafetySystem::timeout_triggered(int board_idx)
{
    log_fault("Heartbeat missed from board %d", board_idx);

    if (missed_cb) {
        missed_cb();
    }
}

// --- Private helpers ---

int HeartbeatSafetySystem::is_board_disabled(Node board)
{
    for (int i = 0; i < ALWAYS_DISABLED_BOARDS_COUNT; i++) {
        if (ALWAYS_DISABLED_BOARDS[i] == board) return 2;
    }
    for (int i = 0; i < DISABLED_BOARDS_COUNT; i++) {
        if (DISABLED_BOARDS[i] == board) return 1;
    }
    return 0;
}