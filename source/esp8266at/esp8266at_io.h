/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP8266AT_IO_H_
#define ESP8266AT_IO_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*!
 * @file esp8266at_uart.h
 *
 * @brief ESP8266 AT UART API
 *
 * ESP8266 module과 AT 명령 통신을 하기 위한 UART API를 정의합니다.
 */

#include <esp8266at/esp8266at_type.h>

esp8266at_err_t esp8266at_io_init(esp8266at_t *esp8266at);
esp8266at_err_t esp8266at_io_deinit(esp8266at_t *esp8266at);

esp8266at_err_t esp8266at_io_read_clear(esp8266at_t *esp8266at);
esp8266at_err_t esp8266at_io_read_clear_timedms(esp8266at_t *esp8266at, uint32_t timeoutms);
esp8266at_err_t esp8266at_io_read_clear_advan(esp8266at_t *esp8266at, uint16_t io_option, uint32_t timeoutms);

esp8266at_err_t esp8266at_io_read(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read);
esp8266at_err_t esp8266at_io_read_timedms(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read, uint32_t timeoutms);
esp8266at_err_t esp8266at_io_read_advan(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read, uint16_t io_option, uint32_t timeoutms);

esp8266at_err_t esp8266at_io_write(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written);
esp8266at_err_t esp8266at_io_write_timedms(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written, uint32_t timeoutms);
esp8266at_err_t esp8266at_io_write_advan(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written, uint16_t io_option, uint32_t timeoutms);

#ifdef	__cplusplus
}
#endif

#endif /* ESP8266AT_IO_H_ */

