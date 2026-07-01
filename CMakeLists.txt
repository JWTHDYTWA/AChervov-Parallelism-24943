cmake_minimum_required(VERSION 3.20)
project(Superbuild NONE)

include(ExternalProject)

ExternalProject_Add(task_2_1
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Task_2_1
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Task_2_1/build
    INSTALL_COMMAND "" # Пропускаем установку, если не требуется
)

ExternalProject_Add(task_2_2
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Task_2_2
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Task_2_2/build
    INSTALL_COMMAND ""
)

ExternalProject_Add(task_2_3
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Task_2_3
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Task_2_3/build
    INSTALL_COMMAND ""
)