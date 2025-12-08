#include "platforms/rtc_io_interface.h"
#include <stdio.h> // For printf

namespace IAQ_RTC {
    void init_rtc() {

        printf("Mock RTC initialized.\n");
    }

    bool get_rtc_time(datetime_t *t) {
  
        if (t) {
            t->year  = 2024;
            t->month = 10;
            t->day   = 30;
            t->dotw  = 4; // Thursday
            t->hour  = 12;
            t->min   = 0;
            t->sec   = 0;
            return true;
        }
        return false;
    }

    void set_rtc_time(datetime_t *t) {
        printf("Mock RTC time set.\n");
    }
}
