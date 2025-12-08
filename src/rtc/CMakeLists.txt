# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rtc C CXX ASM)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/rtc.cpp)

if(PICO_PLATFORM STREQUAL "rp2040" OR PICO_PLATFORM STREQUAL "rp2350")
    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/platforms/pico/rtc_io_interface.cpp)
elseif(PICO_PLATFORM STREQUAL "host")
    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/platforms/host/rtc_io_interface.cpp)
endif()

set(LINK_LIBRARIES
    pico_stdlib
    hardware_i2c
    DS3231
    )

add_library(${PROJECT_NAME}
    ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IOT_FIRMWARE_ROOT_DIR}/third_party/RP2040DS3231RTC/src
)

target_link_libraries(${PROJECT_NAME}
    ${LINK_LIBRARIES}
)


