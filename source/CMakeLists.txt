#
# Copyright (c) 2020 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.3)

include("${CMAKE_CURRENT_LIST_DIR}/../../ubinos/config/common.cmake")

project(esp8266at_wrapper)

project_begin()
project_add_library(ubinos)
project_add_library(stm32cubef2_wrapper)
project_end()


