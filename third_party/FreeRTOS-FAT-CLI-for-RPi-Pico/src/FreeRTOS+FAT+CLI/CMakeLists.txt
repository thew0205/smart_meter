add_library(FreeRTOS+FAT+CLI INTERFACE)


set(SRC_FILES
        ../Lab-Project-FreeRTOS-FAT/ff_crc.c
        ../Lab-Project-FreeRTOS-FAT/ff_dir.c
        ../Lab-Project-FreeRTOS-FAT/ff_error.c
        ../Lab-Project-FreeRTOS-FAT/ff_fat.c
        ../Lab-Project-FreeRTOS-FAT/ff_file.c
        ../Lab-Project-FreeRTOS-FAT/ff_format.c
        ../Lab-Project-FreeRTOS-FAT/ff_ioman.c
        ../Lab-Project-FreeRTOS-FAT/ff_locking.c
        ../Lab-Project-FreeRTOS-FAT/ff_memory.c
        ../Lab-Project-FreeRTOS-FAT/ff_stdio.c
        ../Lab-Project-FreeRTOS-FAT/ff_string.c
        ../Lab-Project-FreeRTOS-FAT/ff_sys.c
        ../Lab-Project-FreeRTOS-FAT/ff_time.c
)

if(PICO_PLATFORM STREQUAL "rp2040" OR PICO_PLATFORM STREQUAL "rp2350")
        list(APPEND SRC_FILES
                portable/RP2040/dma_interrupts.c
                portable/RP2040/ff_sddisk.c
                portable/RP2040/sd_card.c
                portable/RP2040/SPI/sd_card_spi.c
                portable/RP2040/SPI/sd_spi.c
                portable/RP2040/SPI/my_spi.c
                portable/RP2040/SDIO/sd_card_sdio.c
                portable/RP2040/SDIO/rp2040_sdio.c
                src/crash.c
                src/crc.c
                src/ff_utils.c
                src/file_stream.c
                # src/freertos_callbacks.c
                src/FreeRTOS_strerror.c
                src/FreeRTOS_time.c
                src/my_debug.c
                src/sd_timeouts.c
                src/util.c
        )

elseif(PICO_PLATFORM STREQUAL "host")
        list(APPEND SRC_FILES
                ../Lab-Project-FreeRTOS-FAT/portable/common/ff_ramdisk.c
                ../Lab-Project-FreeRTOS-FAT/portable/common/ff_ramdisk.h
                ../Lab-Project-FreeRTOS-FAT/portable/common/ff_sddisk.h
                ../Lab-Project-FreeRTOS-FAT/portable/linux/ff_sddisk.c
                ../Lab-Project-FreeRTOS-FAT/portable/linux/FreeRTOS_time.c
                ../Lab-Project-FreeRTOS-FAT/portable/linux/ff_utils.c
                ../Lab-Project-FreeRTOS-FAT/portable/linux/sd_card.c
                ../Lab-Project-FreeRTOS-FAT/portable/linux/my_debug.c
                ../Lab-Project-FreeRTOS-FAT/portable/linux/util.c
                ../Lab-Project-FreeRTOS-FAT/portable/linux/FreeRTOS_strerror.c
                ../Lab-Project-FreeRTOS-FAT/portable/linux/file_stream.c
        )
endif()


target_sources(FreeRTOS+FAT+CLI INTERFACE
        ${SRC_FILES}

)

set(LINK_LIBS)

if(PICO_PLATFORM STREQUAL "rp2040" OR PICO_PLATFORM STREQUAL "rp2350")
        list(APPEND LINK_LIBS
                cmsis_core
                FreeRTOS-Kernel
                FreeRTOS-Kernel-Heap4
                hardware_adc
                hardware_clocks
                hardware_dma
                hardware_pio
                #hardware_rtc
                hardware_spi
                hardware_timer
                pico_aon_timer
                pico_multicore
                pico_stdlib
                pico_sync
        )
elseif(PICO_PLATFORM STREQUAL "host")
        list(APPEND LINK_LIBS
                pico_stdlib
                FreeRTOS-Kernel
                FreeRTOS-Kernel-Heap4
                pico_stdio
        )
endif()

target_link_libraries(FreeRTOS+FAT+CLI INTERFACE
        ${LINK_LIBS}
)



set(INCLUDE_DIRS
        ../Lab-Project-FreeRTOS-FAT/include/
)

if(PICO_PLATFORM STREQUAL "rp2040" OR PICO_PLATFORM STREQUAL "rp2350")
        list(APPEND INCLUDE_DIRS 
        include/
        portable/RP2040/)
elseif(PICO_PLATFORM STREQUAL "host")
        list(APPEND INCLUDE_DIRS 
        ../Lab-Project-FreeRTOS-FAT/portable/common/
        ../Lab-Project-FreeRTOS-FAT/portable/linux/include
        )

endif()

target_include_directories(FreeRTOS+FAT+CLI INTERFACE
        ${INCLUDE_DIRS}
)
