#
# Copyright (c) 2021 Sung Ho Park and CSOS
#
# SPDX-License-Identifier: Apache-2.0
#

# {ubinos_config_type: [buildable, cmake, app

set(INCLUDE__APP TRUE)
set(APP__NAME "esp8266at_tester")

set_cache(UBINOS__UBICLIB__EXCLUDE_CLI FALSE BOOL)

set_cache(UBINOS__UBIK__TICK_TYPE "RTC" STRING)

set_cache(UBINOS__BSP__DTTY_TYPE "EXTERNAL" STRING)
set_cache(STM32CUBEL4__DTTY_STM32_UART_ENABLE TRUE BOOL)

set_cache(UBINOS__BSP__STM32_DTTY_USARTx_INSTANCE_NUMBER "3" STRING)

include(${PROJECT_UBINOS_DIR}/config/ubinos_nucleol476rg.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubel4_wrapper/config/stm32cubel4.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubel4_extension/config/stm32cubel4_extension.cmake)
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

