#
# Copyright (c) 2020 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

set(INCLUDE__APP TRUE)
set(APP__NAME "esp8266at_tester")

get_filename_component(_tmp_source_dir "${CMAKE_CURRENT_LIST_DIR}/esp8266at_tester" ABSOLUTE)

include_directories(${_tmp_source_dir})

set(PROJECT_APP_SOURCES ${PROJECT_APP_SOURCES} "${_tmp_source_dir}/esp8266at_tester.c")


