# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(smart_meter_src C CXX ASM)


if(NOT DEFINED SMART_METER_ROOT_DIR OR SMART_METER_ROOT_DIR STREQUAL "" OR NOT EXISTS "${SMART_METER_ROOT_DIR}/src")
    message(FATAL_ERROR "SMART_METER_ROOT_DIR must be defined and must point to the root of the project where src folder and the include folders are located")
endif()

set(DS3231_LIB_PATH "${SMART_METER_ROOT_DIR}/third_party/RP2040DS3231RTC")
include(${SMART_METER_ROOT_DIR}/third_party/RP2040DS3231RTC/ds3231_lib_import.cmake)

add_subdirectory(${SMART_METER_ROOT_DIR}/src/fat_sd_card)
add_subdirectory(${SMART_METER_ROOT_DIR}/src/rtc)
add_subdirectory(${SMART_METER_ROOT_DIR}/src/smt_utils)
add_subdirectory(${SMART_METER_ROOT_DIR}/src/pzem004t)


