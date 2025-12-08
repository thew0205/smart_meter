#pragma once

#include "assert_panic.h"

#ifndef CUSTOM_SLEEP_MS
#include "FreeRTOS.h"
#include "task.h"

#define CUSTOM_SLEEP_MS(x) vTaskDelay(pdMS_TO_TICKS(x))
#endif //CUSTOM_SLEEP_MS

#define configASSERT_PANIC(__e) ((__e) ? (void)0 : my_assert_func_panic(__FILE__, __LINE__, __func__, #__e))