#
# Copyright (c) 2021 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

set_cache(UBINOS__UBICLIB__EXCLUDE_CLI FALSE BOOL)

set_cache(UBINOS__UBIK__TICK_TYPE "RTC" STRING)

include(${PROJECT_UBINOS_DIR}/config/ubinos_nucleof207zg_rtctick_external_build.cmake)

include(${PROJECT_LIBRARY_DIR}/esp8266at/config/esp8266at.cmake)

