#include "platforms/rtc_io_interface.h"
#include "DS3231.hpp"

namespace IAQ_RTC
{
// I2C bus settings
#define I2C_PORT i2c1
#define I2C_SDA 6
#define I2C_SCL 7
#define DS3231_I2C_ADDR 0x68

    // Create an instance of the DS3231 class
    DS3231 rtc(I2C_PORT, I2C_SDA, I2C_SCL);
    static bool rtc_present = false;

    void init_rtc()
    {
        uint8_t reg = 0x0F; // Status register
        uint8_t data;
        int ret = i2c_write_blocking(I2C_PORT, DS3231_I2C_ADDR, &reg, 1, true);
        if (ret < 0)
        {
            rtc_present = false;
            return;
        }
        ret = i2c_read_blocking(I2C_PORT, DS3231_I2C_ADDR, &data, 1, false);
        if (ret < 0)
        {
            rtc_present = false;
        }
        else
        {
            rtc_present = true;
        }
        
    }

    bool get_rtc_time(datetime_t *t)
    {
        if (!rtc_present)
        {
            return false;
        }
        t->year = rtc.get_year();
        t->month = rtc.get_mon();
        t->day = rtc.get_day();
        t->dotw = 0; // The new library does not support day of the week
        t->hour = rtc.get_hou();
        t->min = rtc.get_min();
        t->sec = rtc.get_sec();
        return true;
    }

    void set_rtc_time(datetime_t *t)
    {
        if (!rtc_present)
        {
            return;
        }
        rtc.set_year(t->year);
        rtc.set_mon(t->month);
        rtc.set_day(t->day);
        rtc.set_hou(t->hour, false, false);
        rtc.set_min(t->min);
        rtc.set_sec(t->sec);
    }
}
