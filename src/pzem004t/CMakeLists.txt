# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(pzem004t C CXX ASM)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/pzem004t.cpp)


set(LINK_LIBRARIES
    hardware_uart
    hardware_gpio
    hardware_timer
    )

add_library(${PROJECT_NAME}
    ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(${PROJECT_NAME}
    ${LINK_LIBRARIES}
)


