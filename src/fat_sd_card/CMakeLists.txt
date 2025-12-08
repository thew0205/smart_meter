# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

project(fat_sd_card CXX ASM)

set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/fat_sd_card.cpp)

if(PICO_PLATFORM STREQUAL "rp2040" OR PICO_PLATFORM STREQUAL "rp2350")
    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/platforms/pico/hw_config.c)
elseif(PICO_PLATFORM STREQUAL "host")
    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/platforms/host/hw_config.c)
endif()

set(LINK_LIBRARIES  
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    FreeRTOS+FAT+CLI
    )

add_library(${PROJECT_NAME} 
    ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    ${LINK_LIBRARIES}
)