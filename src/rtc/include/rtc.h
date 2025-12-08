#pragma once

#include "pico/util/datetime.h"

namespace IAQ_RTC {
    void init();
    bool get_time(datetime_t *t);
    void set_time(datetime_t *t);
}