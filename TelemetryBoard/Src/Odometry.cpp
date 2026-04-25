#include "SD.h"
#include "Clock.h"
#include "Odometry.h"
#include "log.h"

Odometry::Odometry(SdCard* sd_card) : _sd_card(sd_card) {
    distance_ = _sd_card->read_odometry_data_file();
    time_ = Clock::get_current_time();
}

void Odometry::update(uint32_t current_rpms) {
    uint32_t current_time = Clock::get_current_time();
    uint32_t elapsed_time = current_time - time_;
    time_ = current_time;

    distance_ += static_cast<uint32_t>(current_rpms * elapsed_time); // unit is RPM * ms
    bool success = _sd_card->write_odometry_data(distance_);
    if (!success) {
        log_warn("ODOMETRY: failed to write odometry data");
    }
}

uint32_t Odometry::get_distance() {
    return distance_;
}