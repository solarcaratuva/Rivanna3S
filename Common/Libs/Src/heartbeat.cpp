#include "heartbeat.h"
#include "thread.h"
#include "log.h"

CAN* HeartbeatSafetySystem::can = nullptr;
Callback HeartbeatSafetySystem::missed_cb = nullptr;
HeartbeatSafetySystem::Board HeartbeatSafetySystem::self;

Timeout HeartbeatSafetySystem::timeouts[3];

bool HeartbeatSafetySystem::board_enabled[3] = {
    BOARD_A_ENABLED,
    BOARD_B_ENABLED,
    BOARD_C_ENABLED
};

struct HeartbeatMsg {
    uint16_t id;
};

void HeartbeatSafetySystem::setup(CAN* can_interface,
                                 Callback missed_heartbeat_callback,
                                 Board self_board)
{
    can = can_interface;
    missed_cb = missed_heartbeat_callback;
    self = self_board;

    for (int i = 0; i < static_cast<int>(Board::COUNT); i++) {
        if (!board_enabled[i]) {
            log_info("Board %d is disabled", i);
            continue;
        }

        timeouts[i].attach([i]() {
            timeout_triggered(i);
        }, HEARTBEAT_TIMEOUT_MS);
    }

    static Thread sender_thread;
    sender_thread.start(sender_task);

}

void HeartbeatSafetySystem::sender_task()
{
    Clock clock;

    while (1) {
        HeartbeatMsg msg;
        msg.id = static_cast<uint16_t>(self);

        SerializedCanMessage scm;
        scm.id = 0x100; // example CAN ID
        scm.len = sizeof(msg);
        memcpy(scm.data, &msg, sizeof(msg));

        can->write(scm);

        // Wait 100 ms
        clock.sleep_for(HEARTBEAT_SEND_INTERVAL_MS);
    }
}

void HeartbeatSafetySystem::handle_received_heartbeat(uint16_t board_id)
{
    if (board_id >= static_cast<int>(Board::COUNT)) return;
    if (!board_enabled[board_id]) return;

    timeouts[board_id].refresh();
}

void HeartbeatSafetySystem::timeout_triggered(uint16_t board_id)
{
    log_fault("Missed heartbeat from board %d", board_id);

    if (missed_cb) {
        missed_cb();
    }
}