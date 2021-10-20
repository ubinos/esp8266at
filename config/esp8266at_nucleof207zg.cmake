#
# Copyright (c) 2020 Sung Ho Park and CSOS
#
# SPDX-License-Identifier: Apache-2.0
#

# ubinos_config_info {"name_base": "esp8266at", "build_type": "cmake_ubinos"}

set_cache(UBINOS__UBIK__TICK_TYPE "RTC" STRING)

include(${PROJECT_UBINOS_DIR}/config/ubinos_nucleof207zg.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubef2_wrapper/config/stm32cubef2.cmake)
include(${PROJECT_LIBRARY_DIR}/stm32cubef2_extension/config/stm32cubef2_extension.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/esp8266at.cmake)

####

string(TOLOWER ${UBINOS__BSP__BOARD_MODEL} _temp_board_model)

get_filename_component(_tmp_source_dir "${PROJECT_LIBRARY_DIR}/esp8266at/app/esp8266at_tester" ABSOLUTE)

include_directories(${_tmp_source_dir}/arch/arm/cortexm/${_temp_board_model}/Inc)
include_directories(${_tmp_source_dir})

