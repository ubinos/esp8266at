#
# Copyright (c) 2020 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

set_cache(UBINOS__UBICLIB__EXCLUDE_CLI FALSE BOOL)

set_cache(UBINOS__UBIK__TICK_TYPE "RTC" STRING)

set_cache(UBINOS__BSP__DTTY_TYPE "EXTERNAL" STRING)
set_cache(STM32CUBEF2__DTTY_STM32_UART_ENABLE TRUE BOOL)

include(${PROJECT_UBINOS_DIR}/config/ubinos_nucleof207zg.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubef2_wrapper/config/stm32cubef2.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubef2_extension/config/stm32cubef2_extension.cmake)
include(${PROJECT_LIBRARY_DIR}/esp8266at/config/esp8266at.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/esp8266at_tester.cmake)

####

get_filename_component(_tmp_source_dir "${CMAKE_CURRENT_LIST_DIR}/esp8266at_tester/arch/arm/cortexm/nucleof207zg" ABSOLUTE)

include_directories(${_tmp_source_dir}/Inc)

file(GLOB_RECURSE _tmp_sources
    "${_tmp_source_dir}/Src/*.c"
    "${_tmp_source_dir}/Src/*.cpp"
    "${_tmp_source_dir}/Src/*.S"
    "${_tmp_source_dir}/Src/*.s")

set(PROJECT_APP_SOURCES ${PROJECT_APP_SOURCES} ${_tmp_sources})

