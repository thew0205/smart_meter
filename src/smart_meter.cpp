#include "smt_utils.h"

#include <cstdio>

#include <string>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "memcpy_unique_pointer.h"

#include "fat_sd_card.h"
#include "rtc.h"
#include "pzem004t.h"

#define METER_TO_JSON_FORMAT ("\n=BEGIN={\"voltage\":%0.4f,\"current\": %0.4f,\"power\": %0.4f,\"energy\": %0.4f,\"freq\": %0.4f,\"pf\": %0.4f,\"timestamp\": \"%s\"}==END==\n")
using std::string;

// Start blink task
TaskHandle_t taskSensor;
TaskHandle_t taskStorage;
QueueHandle_t sensorToStorageQueue;

void sensorTask(void *para)
{

    printf("\n=== IoT Air Quality Board ===\n");

    // Initialize hardware modules
    printf("Initialising sensors..\n");
    PZEM004Tv30 meter{uart1, 1};
    meter.init(8, 9, 9600);
    printf("\nSystem Ready.\n\n");
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true)
    {
        memcpy_unique_ptr<string> data_str_p = make_memcpy_unique_ptr<string>("");

        PZEM004Tv30Data data;
        datetime_t dt;
        IAQ_RTC::get_time(&dt);
        char time_buffer[100];

        snprintf(time_buffer,
                 sizeof(time_buffer),
                 "%02d:%02d:%02d-%02d:%02d:%04d",
                 dt.hour, dt.min, dt.sec, dt.day, dt.month, dt.year);

        if (!meter.updateValues(&data))
        {
            data.current = 0.0f;
            data.voltage = 0.0f;
            data.power = 0.0f;
            data.freq = 0.0f;
            data.pf = 0.0f;
            // Set data to last value as it is monotonically increasing.
            data.energy = data.energy;
        }

        int needed_size = sprintf(nullptr, METER_TO_JSON_FORMAT, data.voltage, data.current, data.power, data.energy, data.freq, data.pf, time_buffer);

        data_str_p->resize(needed_size + 1);
        sprintf(data_str_p->data(), METER_TO_JSON_FORMAT, data.voltage, data.current, data.power, data.energy, data.freq, data.pf, time_buffer);

        printf("%s\n", data_str_p->data());
        data_str_p.memcpy_send(nullptr, [](void *, const memcpy_unique_ptr<string> *src)
                               { return xQueueSend(sensorToStorageQueue, src, 0) == pdTRUE; });
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000 * 10));
    }
}

void storageTask(void *para)
{
    configASSERT_PANIC(fat_sd_card_init(false));

    while (1)
    {
        memcpy_unique_ptr<string> data_str_p{};

        data_str_p.memcpy_receive(nullptr, [](memcpy_unique_ptr<string> *dest, const void *const src)
                                  { return xQueueReceive(sensorToStorageQueue, dest, portMAX_DELAY) == pdTRUE; });
        datetime_t dt;
        IAQ_RTC::get_time(&dt);
        char file_name_buffer[60];
        snprintf(file_name_buffer, sizeof(file_name_buffer), "/sd0/meter_data_%02d-%02d-%04d.json", dt.day, dt.month, dt.year);
        FF_FILE *file = fat_sd_card_open(file_name_buffer, "a");
        fat_sd_card_write(data_str_p->c_str(), data_str_p->length(), file);
        fat_sd_card_close(file);
    }
    fat_sd_card_deinit();
}

int main()
{
    // Initialise standard I/O
    stdio_init_all();
    IAQ_RTC::init();

    sensorToStorageQueue = xQueueCreate(1, sizeof(memcpy_unique_ptr<string>));

    configASSERT_PANIC(pdPASS != xTaskCreate(sensorTask, "sensorThread", 1024 * 4, NULL, 2, &taskSensor));

    configASSERT_PANIC(pdPASS != xTaskCreate(storageTask, "storageThread", 1024 * 4, NULL, 2, &taskStorage));

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    return 0;
}