#pragma once

#include "can.h"
#include "Timeout.h"
#include <functional>

using Callback = std::function<void()>;

class HeartbeatSafetySystem {
public:
    static constexpr uint32_t HEARTBEAT_SEND_INTERVAL_MS = 100;

    // How long we WAIT before declaring failure (250 ms)
    static constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 250;

    // Enable / disable boards
    static constexpr bool BOARD_A_ENABLED = true;
    static constexpr bool BOARD_B_ENABLED = true;
    static constexpr bool BOARD_C_ENABLED = false; // example disabled

    enum class Board {
        BOARD_A = 0,
        BOARD_B,
        BOARD_C,
        COUNT
    };

    static void setup(CAN* can_interface,
                      Callback missed_heartbeat_callback,
                      Board self_board);

private:
    static void sender_task();
    static void handle_received_heartbeat(uint16_t board_id);
    static void timeout_triggered(uint16_t board_id);

    static CAN* can;
    static Callback missed_cb;
    static Board self;

    static Timeout timeouts[static_cast<int>(Board::COUNT)];
    static bool board_enabled[static_cast<int>(Board::COUNT)];
};