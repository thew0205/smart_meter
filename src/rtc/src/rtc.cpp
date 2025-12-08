#include "rtc.h"
#include "platforms/rtc_io_interface.h"
#include <stdio.h>

static datetime_t datetime;

namespace IAQ_RTC {
    void init() {
        // Start the RTC
        init_rtc();
        printf("RTC initialised.\n");
    }

    void set_time(datetime_t *t) {
        set_rtc_time(t);
    }

    bool get_time(datetime_t *t) {
        return get_rtc_time(t);
    }
}

