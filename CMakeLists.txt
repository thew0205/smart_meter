# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if(EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(smart_meter LANGUAGES C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1


set(SMART_METER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})


set(FREERTOS_KERNEL_PATH "${SMART_METER_ROOT_DIR}/third_party/FreeRTOS-Kernel")

set(FREERTOS_CONFIG_FILE_DIRECTORY "${SMART_METER_ROOT_DIR}/include" CACHE STRING "Local Config")

if(PICO_PLATFORM STREQUAL "rp2040")
    include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
elseif(PICO_PLATFORM STREQUAL "rp2350-arm-s")
    include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)
endif()

target_link_libraries(FreeRTOS-Kernel INTERFACE pico_stdlib hardware_clocks cmsis_core)

target_sources(FreeRTOS-Kernel INTERFACE ${SMART_METER_ROOT_DIR}/src/platforms/freertos_port.c)

add_subdirectory(${SMART_METER_ROOT_DIR}/third_party/FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI ${CMAKE_BINARY_DIR}/FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI)

add_subdirectory(${SMART_METER_ROOT_DIR}/third_party/memcpy_smart_ptr ${CMAKE_BINARY_DIR}/third_party/memcpy_smart_ptr)
add_subdirectory(src)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/smart_meter.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    smt_utils
    fat_sd_card
    rtc
    pzem004t
    memcpy_smart_ptr
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${SMART_METER_ROOT_DIR}/include
)

#output options
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

#ouputs .bin, .elf, .uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
