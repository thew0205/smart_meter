# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(smt_utils C CXX ASM)

set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/assert_panic.c)

set(LINK_LIBRARIES  
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
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