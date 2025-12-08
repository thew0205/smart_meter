/* sd_card.c
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

/* Standard includes. */
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
//
#include "hw_config.h"  // Hardware Configuration of the SPI and SD Card "objects"
#include "my_debug.h"
#include "util.h"
//
#include "sd_card.h"

#define TRACE_PRINTF(fmt, args...)
// #define TRACE_PRINTF printf

sd_card_t *sd_get_by_name(const char *const name) {
    myASSERT(name);
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->device_name, name)) return sd_get_by_num(i);
    DBG_PRINTF("%s: unknown name %s\n", __func__, name);
    return NULL;
}
sd_card_t *sd_get_by_mount_point(const char *const name) {
    myASSERT(name);
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->mount_point, name)) return sd_get_by_num(i);
    DBG_PRINTF("%s: unknown name %s\n", __func__, name);
    return NULL;
}

/* [] END OF FILE */
