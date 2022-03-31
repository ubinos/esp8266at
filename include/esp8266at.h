/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP8266AT_H_
#define ESP8266AT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * @file esp8266at.h
 *
 * @brief ESP8266 AT API
 *
 * ESP8266 AT API를 정의합니다.
 */

#include <time.h>

#include <esp8266at/esp8266at_type.h>

esp8266at_err_t esp8266at_init(esp8266at_t *esp8266at);

esp8266at_err_t esp8266at_deinit(esp8266at_t *esp8266at);

esp8266at_err_t esp8266at_cmd_at_interactive(esp8266at_t *esp8266at);

esp8266at_err_t esp8266at_cmd_at_test(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_rst(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_gmr(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_e(esp8266at_t *esp8266at, int is_on, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cwmode(esp8266at_t *esp8266at, int mode, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipmux(esp8266at_t *esp8266at, int mode, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cwjap(esp8266at_t *esp8266at, char * ssid, char * passwd, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cwqap(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cifsr(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipstart(esp8266at_t *esp8266at, char *type, char *ip, uint32_t port, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipstart_multiple(esp8266at_t *esp8266at, int id, char *type, char *ip, uint32_t port, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipclose(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipsend(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_ciprecv(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipsntpcfg(esp8266at_t *esp8266at, uint8_t enable, uint8_t timezone, char * sntp_server_addr, char * sntp_server_addr2, char * sntp_server_addr3, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipsntpcfg_q(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_cmd_at_cipsntptime(esp8266at_t *esp8266at, struct tm * tm_ptr, uint32_t timeoutms, uint32_t *remain_timeoutms);

#ifdef __cplusplus
}
#endif

#endif /* ESP8266AT_H_ */

