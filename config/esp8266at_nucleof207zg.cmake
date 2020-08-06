#
# Copyright (c) 2020 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

include(${PROJECT_UBINOS_DIR}/config/ubinos_nucleof207zg.cmake)

include(${PROJECT_LIBRARY_DIR}/stm32cubef2_wrapper/config/stm32cubef2.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/esp8266at.cmake)

####

get_filename_component(_tmp_source_dir "${CMAKE_CURRENT_LIST_DIR}/../app/esp8266at_tester" ABSOLUTE)

include_directories(${_tmp_source_dir})
include_directories(${_tmp_source_dir}/arch/arm/cortexm/nucleof207zg)


