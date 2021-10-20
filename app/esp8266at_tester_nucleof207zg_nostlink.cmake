#
# Copyright (c) 2021 Sung Ho Park and CSOS
#
# SPDX-License-Identifier: Apache-2.0
#

# ubinos_config_info {"name_base": "esp8266at_tester", "build_type": "cmake_ubinos", "app": true}

set_cache(UBINOS__BSP__STM32_RCC_HSE_CONFIG "ON" STRING)

include(${CMAKE_CURRENT_LIST_DIR}/esp8266at_tester_nucleof207zg.cmake)

