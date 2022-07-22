#
# Copyright (c) 2020 Sung Ho Park and CSOS
#
# SPDX-License-Identifier: Apache-2.0
#

set(INCLUDE__ESP8266AT TRUE)
set(PROJECT_UBINOS_LIBRARIES ${PROJECT_UBINOS_LIBRARIES} esp8266at)

set_cache_default(ESP8266AT__LOGM_CATEGORY "SYS00" STRING "logm category of esp8266at")

set_cache_default(ESP8266AT__USE_RESET_PIN FALSE BOOL "Use reset pin")
set_cache_default(ESP8266AT__USE_CHIPSELECT_PIN TRUE BOOL "Use chip select pin")
set_cache_default(ESP8266AT__USE_UART_HW_FLOW_CONTROL FALSE BOOL "Use uart hardware flow control")

set_cache_default(ESP8266AT__USE_WIZFI360_API FALSE BOOL "Use WizFi360 API")
