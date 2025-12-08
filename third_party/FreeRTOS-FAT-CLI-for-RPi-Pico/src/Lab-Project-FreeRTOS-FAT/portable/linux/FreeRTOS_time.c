#include "FreeRTOS.h"

#include "ff_time.h"

#include <time.h>

time_t FreeRTOS_time(time_t *pxTime)
{
    time_t now = time(NULL); // POSIX system time in seconds

    if (pxTime != NULL)
    {
        *pxTime = now;
    }

    return now;
}