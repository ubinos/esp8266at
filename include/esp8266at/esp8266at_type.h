/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP8266AT_TYPE_H_
#define ESP8266AT_TYPE_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*!
 * @file esp8266at_type.h
 *
 * @brief ESP8266 AT Type
 *
 * ESP8266 module과 AT 명령 통신을 하기 위한 형들을 정의합니다.
 */

#include <stdint.h>

typedef enum {
	ESP8266AT_OK = 0,
	ESP8266AT_ERROR,
	ESP8266AT_TIMEOUT,
	ESP8266AT_BUSY,
	ESP8266AT_IO_ERROR,
} esp8266at_err_t;

#define ESP8266AT_IO_OPTION__TIMED 0x0001

#ifdef	__cplusplus
}
#endif

#endif /* ESP8266AT_TYPE_H_ */

