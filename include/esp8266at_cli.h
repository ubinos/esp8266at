/*
 * Copyright (c) 2021 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP8266AT_CLI_H_
#define ESP8266AT_CLI_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * @file esp8266at_cli.h
 *
 * @brief ESP8266 AT CLI API
 *
 * ESP8266 AT CLI API를 정의합니다.
 */

#include <esp8266at/esp8266at_type.h>

int esp8266at_cli_at(esp8266at_t *esp8266at, char *str, int len, void *arg);

void esp8266at_cli_at_interactive(esp8266at_t *esp8266at);

void esp8266at_cli_at_test(esp8266at_t *esp8266at);
void esp8266at_cli_at_reset(esp8266at_t *esp8266at);
void esp8266at_cli_at_query_version(esp8266at_t *esp8266at);
void esp8266at_cli_at_query_dns(esp8266at_t *esp8266at);
void esp8266at_cli_at_query_sntpcfg(esp8266at_t *esp8266at);
void esp8266at_cli_at_query_sntptime(esp8266at_t *esp8266at);

int esp8266at_cli_at_config(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_config_echo(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_config_wmode(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_config_ipmux(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_config_ap(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_config_dns(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_config_sntpcfg(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_config_mqttusercfg(esp8266at_t *esp8266at, char *str, int len, void *arg);

int esp8266at_cli_at_ap(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_ap_join(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_ap_quit(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_ap_query_ip(esp8266at_t *esp8266at, char *str, int len, void *arg);

int esp8266at_cli_at_conn(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_conn_open(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_conn_close(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_conn_send(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_conn_recv(esp8266at_t *esp8266at, char *str, int len, void *arg);

int esp8266at_cli_at_mqtt(esp8266at_t *esp8266at, char *str, int len, void *arg);
#if (ESP8266AT__USE_WIZFI360_API == 1)
int esp8266at_cli_at_mqtt_topic(esp8266at_t *esp8266at, char *str, int len, void *arg);
#else
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */
int esp8266at_cli_at_mqtt_open(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_mqtt_close(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_mqtt_pub(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_mqtt_sub(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_mqtt_sublist(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_mqtt_unsub(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_at_mqtt_subget(esp8266at_t *esp8266at, char *str, int len, void *arg);

int esp8266at_cli_rdate(esp8266at_t *esp8266at, char *str, int len, void *arg);
int esp8266at_cli_echo_client(esp8266at_t *esp8266at, char *str, int len, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* ESP8266AT_CLI_H_ */

