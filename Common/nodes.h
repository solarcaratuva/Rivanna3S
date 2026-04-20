#include <cstdio>
#include <cstdint>
#include "Rivanna3SCanStructs.h"

// every node in the Rivanna3S system
enum class Node {
    BottomDistBoard = 0,
    MotorBoard = 1,
    RelayBoard = 2,
    TelemetryBoard = 3,
    TopDistBoard = 4,
    WheelBoard = 5,

    BMS = 6,
    MotorController = 7,

    Undefined = 255, // sentinel value for unrecognized node
};
#define NUM_NODES 8

static void node_to_str(Node node, char* out_str, uint8_t out_str_size) {
    switch (node) {
        case Node::BottomDistBoard:     snprintf(out_str, out_str_size, "BottomDistBoard"); break;
        case Node::MotorBoard:          snprintf(out_str, out_str_size, "MotorBoard"); break;
        case Node::RelayBoard:          snprintf(out_str, out_str_size, "RelayBoard"); break;
        case Node::TelemetryBoard:      snprintf(out_str, out_str_size, "TelemetryBoard"); break;
        case Node::TopDistBoard:        snprintf(out_str, out_str_size, "TopDistBoard"); break;
        case Node::WheelBoard:          snprintf(out_str, out_str_size, "WheelBoard"); break;
        case Node::BMS:                 snprintf(out_str, out_str_size, "BMS"); break;
        case Node::MotorController:     snprintf(out_str, out_str_size, "MotorController"); break;
        default:                        snprintf(out_str, out_str_size, "Undefined"); break;
    }
}
