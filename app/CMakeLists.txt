#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

include_directories(${CMAKE_SOURCE_DIR}/include)

target_sources(app PRIVATE
  ${CMAKE_SOURCE_DIR}/src/adc.c
  ${CMAKE_SOURCE_DIR}/src/main.c
  ${CMAKE_SOURCE_DIR}/src/bsp.c
  ${CMAKE_SOURCE_DIR}/src/event_task.c
  ${CMAKE_SOURCE_DIR}/src/control_task.c
  ${CMAKE_SOURCE_DIR}/src/sensor_task.c
)
