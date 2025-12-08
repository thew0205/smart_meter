/* sd_card.h
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

   http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/

// Note: The model used here is one FatFS per SD card.
// Multiple partitions on a card are not supported.

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
//
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "ff_headers.h"
#include "semphr.h"
//

#ifdef __cplusplus
extern "C"
{
#endif

    typedef uint8_t BYTE;
    /* Status of Disk Functions */
    typedef BYTE DSTATUS;

    typedef struct sd_card_state_t
    {
        DSTATUS m_Status; // Card status
        uint32_t sectors; // Assigned dynamically

        SemaphoreHandle_t mutex;        // Guard semaphore, assigned dynamically
        StaticSemaphore_t mutex_buffer; // Guard semaphore storage, assigned dynamically
        TaskHandle_t owner;             // Assigned dynamically
        FF_Disk_t ff_disk;              // FreeRTOS+FAT "disk" using this device
    } sd_card_state_t;

    typedef struct sd_file_if_t
    {
        FILE *file;
        const char *file_name;

    } sd_file_if_t;

    typedef struct sd_card_t sd_card_t;

    // "Class" representing SD Cards
    struct sd_card_t
    {
        const char *device_name;
        const char *mount_point; // Must be a directory off the file system's root directory and
                                 // must be an absolute path that starts with a forward slash (/)
        sd_file_if_t file_if;
        bool use_card_detect;
        uint card_detect_gpio;   // Card detect; ignored if !use_card_detect
        uint card_detected_true; // Varies with card socket; ignored if !use_card_detect
        bool card_detect_use_pull;
        bool card_detect_pull_hi;
        size_t cache_sectors;

        /* The following fields are state variables and not part of the configuration.
        They are dynamically assigned. */
        sd_card_state_t state;
    };

    sd_card_t *sd_get_by_name(const char *const name);
    sd_card_t *sd_get_by_mount_point(const char *const name);

#ifdef __cplusplus
}
#endif

/* [] END OF FILE */
