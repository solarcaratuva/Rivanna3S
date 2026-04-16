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

    Node sender = (Node)(hb.source);
    int  idx    = (int)(sender);

    if (is_board_disabled(sender))   
        return;

    timeouts[idx].refresh();
}

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
        Node board = (Node)(i);

        if (board == self_board) {
            continue;
        }

        if (is_board_disabled(board) == 1) {
            char node_str[32];
            node_to_str(board, node_str, sizeof(node_str));
            log_info("Heartbeat: %s disabled", node_str); // only log the boards that are not permanently disabled
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

    Heartbeat hb{};
    hb.source = (uint8_t)(self_board);

    SerializedCanMessage scm{};
    hb.serialize(&scm);

    while (true) {
        can->write(&scm);

        clock.sleep_for(HEARTBEAT_SEND_INTERVAL_MS);
    }
}

// --- Timeout handler ---

void HeartbeatSafetySystem::timeout_triggered(int board_idx)
{
    Node board = (Node)(board_idx);

    char node_str[32];
    node_to_str(board, node_str, sizeof(node_str));
    log_fault("Heartbeat missed from %d", node_str);

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