#include "heartbeat.h"
#include "thread.h"
#include "clock.h"
#include "log.h"
#include <cstring>

uint16_t heartbeat_can_m_id = 0x100;

CanInterface* HeartbeatSafetySystem::can = nullptr;
Callback HeartbeatSafetySystem::missed_cb = nullptr;
HeartbeatSafetySystem::Board HeartbeatSafetySystem::self;

Timeout HeartbeatSafetySystem::timeouts[static_cast<int>(Board::COUNT)];

bool HeartbeatSafetySystem::board_enabled[static_cast<int>(Board::COUNT)] = {
    BOARD_MOTOR_ENABLE,
    BOARD_RELAY_ENABLE,
    BOARD_TELEMETRY_ENABLE,
    BOARD_TOPDIST_ENABLE,
    BOARD_BOTDIST_ENABLE
};

struct HeartbeatMsg {
    uint16_t id;
};

void HeartbeatSafetySystem::heartbeat_can_callback(const SerializedCanMessage &msg)
{
    if (msg.len < sizeof(uint16_t)) return;

    HeartbeatMsg hb{};
    std::memcpy(&hb, msg.data, sizeof(hb));

    handle_received_heartbeat(hb.id);
}

void HeartbeatSafetySystem::setup(CanInterface* can_interface,
                                 Callback missed_heartbeat_callback,
                                 Board self_board)
{
    can = can_interface;
    missed_cb = missed_heartbeat_callback;
    self = self_board;

    // Register CAN callback for heartbeat messages
    can->register_callback(heartbeat_can_m_id, HeartbeatSafetySystem::heartbeat_can_callback);

    // Setup timeouts
    for (int i = 0; i < static_cast<int>(Board::COUNT); i++) {
        if (!board_enabled[i]) {
            log_info("Heartbeat: Board %d disabled", i);
            continue;
        }

        timeouts[i].attach([i]() {
            timeout_triggered(i);
        }, HEARTBEAT_TIMEOUT_MS);
    }

    // Sender thread
    static Thread sender_thread;
    sender_thread.start(sender_task);
}

void HeartbeatSafetySystem::sender_task()
{
    Clock clock;

    while (true) {
        HeartbeatMsg msg{};
        msg.id = static_cast<uint16_t>(self);

        SerializedCanMessage scm{};
        scm.id = heartbeat_can_m_id;
        scm.len = sizeof(msg);

        std::memcpy(scm.data, &msg, sizeof(msg));

        can->write(&scm);

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
    log_fault("Heartbeat missed from board %d", board_id);

    if (missed_cb) {
        missed_cb();
    }
}