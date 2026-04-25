#include "SD.h"


class Odometry {
    public:
        Odometry(SdCard* sd_card);
        void update(uint32_t current_rpms); 
        uint32_t get_distance();
    private:
        SdCard* _sd_card;
        float distance_;
        uint32_t time_;
};