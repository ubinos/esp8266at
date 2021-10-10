#
# Copyright (c) 2020 Sung Ho Park and CSOS
#
# SPDX-License-Identifier: Apache-2.0
#

# {ubinos_config_type: [buildable, cmake, app

set(INCLUDE__APP TRUE)
set(APP__NAME "esp8266at_tester")

set_cache(UBINOS__UBICLIB__EXCLUDE_CLI FALSE BOOL)

set_cache(UBINOS__UBIK__TICK_TYPE "RTC" STRING)

set_cache(UBINOS__BSP__DTTY_TYPE "EXTERNAL" STRING)
set_cache(SEGGERRTT__DTTY_ENABLE TRUE BOOL)

set_cache(UBINOS__BSP__STM32_RCC_HSE_CONFIG "ON" STRING)
set_cache(UBINOS__BSP__STM32_HSE_VALUE "25000000U" STRING)

set_cache(UBINOS__BSP__OPENOCD_CONFIG_FILE "" PATH)
set_cache(UBINOS__BSP__GDBSCRIPT_FILE_LOAD "${PROJECT_UBINOS_DIR}/resource/ubinos/bsp/arch/arm/cortexm/gdb_flash_load.gdb" PATH)
set_cache(UBINOS__BSP__GDBSCRIPT_FILE_RESET "${PROJECT_UBINOS_DIR}/resource/ubinos/bsp/arch/arm/cortexm/gdb_flash_reset.gdb" PATH)

include(${PROJECT_UBINOS_DIR}/config/ubinos_nucleof207zg.cmake)
include(${PROJECT_LIBRARY_DIR}/seggerrtt_wrapper/config/seggerrtt.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubef2_wrapper/config/stm32cubef2.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubef2_extension/config/stm32cubef2_extension.cmake)
include(${PROJECT_LIBRARY_DIR}/esp8266at/config/esp8266at.cmake)

get_filename_component(_tmp_source_dir "${CMAKE_CURRENT_LIST_DIR}/${APP__NAME}" ABSOLUTE)
string(TOLOWER ${UBINOS__BSP__BOARD_MODEL} _temp_board_model)

include_directories(${_tmp_source_dir}/arch/arm/cortexm/${_temp_board_model}/Inc)
include_directories(${_tmp_source_dir})

file(GLOB_RECURSE _tmp_sources
    "${_tmp_source_dir}/*.c"
    "${_tmp_source_dir}/*.cpp"
    "${_tmp_source_dir}/*.cc"
    "${_tmp_source_dir}/*.S"
    "${_tmp_source_dir}/*.s")

set(PROJECT_APP_SOURCES ${PROJECT_APP_SOURCES} ${_tmp_sources})

