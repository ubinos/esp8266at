/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP8266AT_H_
#define ESP8266AT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*!
 * @file esp8266at.h
 *
 * @brief ESP8266 AT API
 *
 * ESP8266 AT API를 정의합니다.
 */

#include <esp8266at/esp8266at_type.h>

esp8266at_err_t esp8266at_init(void);

esp8266at_err_t esp8266at_deinit(void);

esp8266at_err_t esp8266at_cmd_at_test(uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_rst(uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_gmr(uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_e(int is_on, uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cwmode(int mode, uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipmux(int mode, uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cwjap(char * ssid, char * passwd, uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cwqap(uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cifsr(uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipstart(char *type, char *ip, uint32_t port, uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipstart_multiple(int id, char *type, char *ip, uint32_t port, uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipclose(uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipsend(uint8_t *buffer, uint32_t length, uint32_t timeoutms);

esp8266at_err_t esp8266at_cmd_at_ciprecv(uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms);

#ifdef	__cplusplus
}
#endif

#endif /* ESP8266AT_H_ */

