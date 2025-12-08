
/* hw_config.c
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

/*

This file should be tailored to match the hardware design.

See
https://github.com/carlk3/FreeRTOS-FAT-CLI-for-RPi-Pico

There should be one element of the spi[] array for each RP2040 hardware SPI used.

There should be one element of the spi_ifs[] array for each SPI interface object.
* Each element of spi_ifs[] must point to an spi_t instance with member "spi".

There should be one element of the sdio_ifs[] array for each SDIO interface object.

There should be one element of the sd_cards[] array for each SD card slot.
* Each element of sd_cards[] must point to its interface with spi_if_p or sdio_if_p.
*/

//
#include "hw_config.h"

// Hardware Configuration of SPI "object"
static spi_t spi  = {  // One for each RP2040 SPI component used
    .hw_inst = spi0,  // SPI component
    .sck_gpio = 2,  // GPIO number (not Pico pin number)
    .mosi_gpio = 3,
    .miso_gpio = 4,
    .set_drive_strength = true,
    .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
    .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_12MA,
    .no_miso_gpio_pull_up = true,
    .DMA_IRQ_num = DMA_IRQ_0,
    .use_exclusive_DMA_IRQ_handler = true,
    // .baud_rate = 125 * 1000 * 1000 / 8  // 15625000 Hz
    // .baud_rate = 125 * 1000 * 1000 / 6  // 20833333 Hz
    // .baud_rate = 125 * 1000 * 1000 / 4  // 31250000 Hz
    .baud_rate = 1 * 1000 * 1000           // 1000000 Hz

};

/* SPI Interface */
static sd_spi_if_t spi_if = {
        .spi = &spi,  // Pointer to the SPI driving this card
        .ss_gpio = 5,     // The SPI slave select GPIO for this SD card
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA
};

/* Configuration of the SD Card socket object */
static sd_card_t sd_card = {
    // "device_name" is arbitrary:
    .device_name = "sd0",
    // "mount_point" must be a directory off the file system's root directory and must be an absolute path:
    .mount_point = "/sd0",
    .type = SD_IF_SPI,
    .spi_if_p = &spi_if  // Pointer to the SPI interface driving this card
    // .spi_if_p = NULL  // Pointer to the SPI interface driving this card

};

/* ********************************************************************** */

size_t sd_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num) {
        return &sd_card;
    } else {
        return NULL;
    }
}

/* [] END OF FILE */
