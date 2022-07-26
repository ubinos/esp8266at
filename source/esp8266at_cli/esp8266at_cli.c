/*
 * Copyright (c) 2021 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <esp8266at.h>
#include <esp8266at_cli.h>

#if (INCLUDE__ESP8266AT == 1)

#include "../../source/esp8266at/esp8266at_io.h"

#undef LOGM_CATEGORY
#define LOGM_CATEGORY ESP8266AT__LOGM_CATEGORY

#define ESP8266AT_RECV_BUFFER_SIZE 1500
#define ESP8266AT_MQTT_MSG_BUFFER_SIZE 512

static uint8_t _recv_buf[ESP8266AT_RECV_BUFFER_SIZE];
static uint8_t _mqtt_msg_buf[ESP8266AT_MQTT_MSG_BUFFER_SIZE];

static uint32_t _timeoutms = 10000;

int esp8266at_cli_at(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    char *tmpstr;
    int tmplen;
    char *cmd = NULL;
    int cmdlen = 0;

    tmpstr = str;
    tmplen = len;

    do
    {
        cmd = "i";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_interactive(esp8266at);
            r = 0;
            break;
        }

        cmd = "test";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_test(esp8266at);
            r = 0;
            break;
        }

        cmd = "reset";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_reset(esp8266at);
            r = 0;
            break;
        }

        cmd = "version";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_query_version(esp8266at);
            r = 0;
            break;
        }

        cmd = "dns";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_query_dns(esp8266at);
            r = 0;
            break;
        }

        cmd = "sntp";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_query_sntpcfg(esp8266at);
            r = 0;
            break;
        }

        cmd = "time";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_query_sntptime(esp8266at);
            r = 0;
            break;
        }

        cmd = "c ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "ap ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_ap(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "conn ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_conn(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "mqtt ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        break;
    } while (1);

    return r;
}

void esp8266at_cli_at_interactive(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    printf("start interactive mode (press ESC key to exit)\n");
    err = esp8266at_cmd_at_interactive(esp8266at);
    printf("result : err = %d\n", err);
}

void esp8266at_cli_at_test(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_test(esp8266at, _timeoutms, NULL);
    printf("result : err = %d\n", err);
}

void esp8266at_cli_at_reset(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_reset(esp8266at);
    printf("result : err = %d\n", err);
}

void esp8266at_cli_at_query_version(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_gmr(esp8266at, _timeoutms, NULL);
    printf("result : err = %d, version = %s\n", err, esp8266at->version);
}

void esp8266at_cli_at_query_dns(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_cipdns_q(esp8266at, _timeoutms, NULL);
    printf("result : err = %d, enable = %d", err, esp8266at->dns_enable);
    for (int i = 0; i < ESP8266AT_DNS_SERVER_MAX; i++)
    {
        if (strlen(esp8266at->dns_server_addr[i]) > 0)
        {
            printf(", server[%d] = %s", i, esp8266at->dns_server_addr[i]);
        }
    }
    printf("\n");
}

void esp8266at_cli_at_query_sntpcfg(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_cipsntpcfg_q(esp8266at, _timeoutms, NULL);
    printf("result : err = %d, enable = %d, timezone=%d", err, esp8266at->sntp_enable, esp8266at->sntp_timezone);
    for (int i = 0; i < ESP8266AT_SNTP_SERVER_MAX; i++)
    {
        if (strlen(esp8266at->sntp_server_addr[i]) > 0)
        {
            printf(", server[%d] = %s", i, esp8266at->sntp_server_addr[i]);
        }
    }
    printf("\n");
}

void esp8266at_cli_at_query_sntptime(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    struct tm tm_data;

    err = esp8266at_cmd_at_cipsntptime(esp8266at, &tm_data, _timeoutms, NULL);
    printf("result : err = %d, year = %d, mon = %d, mday = %d, wday = %d, hour = %d, min = %d, sec = %d\n", err, 
        tm_data.tm_year + 1900, tm_data.tm_mon + 1, tm_data.tm_mday, tm_data.tm_wday, tm_data.tm_hour, tm_data.tm_min, tm_data.tm_sec);
}

int esp8266at_cli_at_config(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    char *tmpstr;
    int tmplen;
    char *cmd = NULL;
    int cmdlen = 0;

    tmpstr = str;
    tmplen = len;

    do
    {
        cmd = "echo ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config_echo(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "wmode ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config_wmode(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "ipmux ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config_ipmux(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "ap ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config_ap(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "dns ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config_dns(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "sntp ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config_sntpcfg(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "mqtt ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config_mqttusercfg(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_config_echo(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    char *tmpstr;
    int tmplen;
    char *cmd = NULL;
    int cmdlen = 0;

    esp8266at_err_t err;
    int is_on;

    tmpstr = str;
    tmplen = len;

    do
    {
        cmd = "on";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            is_on = 1;
        }
        else
        {
            cmd = "off";
            cmdlen = strlen(cmd);
            if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
            {
                is_on = 0;
            }
            else
            {
                r = -1;
                break;
            }
        }

        err = esp8266at_cmd_at_e(esp8266at, is_on, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_config_wmode(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    int mode;

    mode = atoi(str);

    err = esp8266at_cmd_at_cwmode(esp8266at, mode, _timeoutms, NULL);
    printf("result : err = %d\n", err);
    r = 0;

    return r;
}

int esp8266at_cli_at_config_ipmux(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    int mode;

    mode = atoi(str);

    err = esp8266at_cmd_at_cipmux(esp8266at, mode, _timeoutms, NULL);
    printf("result : err = %d\n", err);
    r = 0;

    return r;
}

int esp8266at_cli_at_config_ap(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    do
    {
        sscanf(str, "%s %s", esp8266at->ssid, esp8266at->passwd);
        printf("result : err = %d\n", 0);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_config_dns(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    int enable = 0;
    char dns_server_addr[3][ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX];

    do
    {
        memset(dns_server_addr, 0, 3 * ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX);
        sscanf(str, "%d %s %s %s", &enable,
            dns_server_addr[0], dns_server_addr[1], dns_server_addr[2]);

        r = esp8266at_cmd_at_cipdns(esp8266at, enable,
            dns_server_addr[0], dns_server_addr[1], dns_server_addr[2],
            _timeoutms, NULL);

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_config_sntpcfg(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    int enable = 0;
    int timezone = 0;
    char sntp_server_addr[3][ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX];

    do
    {
        memset(sntp_server_addr, 0, 3 * ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX);
        sscanf(str, "%d %d %s %s %s", &enable, &timezone, 
            sntp_server_addr[0], sntp_server_addr[1], sntp_server_addr[2]);

        r = esp8266at_cmd_at_cipsntpcfg(esp8266at, enable, timezone,
            sntp_server_addr[0], sntp_server_addr[1], sntp_server_addr[2],
            _timeoutms, NULL);

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_config_mqttusercfg(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    int mqtt_scheme = 1;
    char mqtt_client_id[ESP8266AT_MQTT_CLIENT_ID_LENGTH_MAX];
    char mqtt_username[ESP8266AT_MQTT_USERNAME_LENGTH_MAX];
    char mqtt_passwd[ESP8266AT_MQTT_PASSWD_LENGTH_MAX];

    do
    {
        memset(mqtt_client_id, 0, ESP8266AT_MQTT_CLIENT_ID_LENGTH_MAX);
        memset(mqtt_username, 0, ESP8266AT_MQTT_USERNAME_LENGTH_MAX);
        memset(mqtt_passwd, 0, ESP8266AT_MQTT_PASSWD_LENGTH_MAX);

#if (ESP8266AT__USE_WIZFI360_API == 1)
        sscanf(str, "%s %s %s",
            mqtt_client_id, mqtt_username, mqtt_passwd);
#else
        sscanf(str, "%d %s %s %s", &mqtt_scheme,
            mqtt_client_id, mqtt_username, mqtt_passwd);
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

        r = esp8266at_cmd_at_mqttusercfg(esp8266at, mqtt_scheme,
            mqtt_client_id, mqtt_username, mqtt_passwd,
            _timeoutms, NULL);

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_ap(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    char *tmpstr;
    int tmplen;
    char *cmd = NULL;
    int cmdlen = 0;

    tmpstr = str;
    tmplen = len;

    do
    {
        cmd = "join";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_ap_join(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "quit";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_ap_quit(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "ip";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_ap_query_ip(esp8266at, tmpstr, tmplen, arg);
            r = 0;
            break;
        }

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_ap_join(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;

    do
    {
        err = esp8266at_cmd_at_cwjap(esp8266at, esp8266at->ssid, esp8266at->passwd, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_ap_quit(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    esp8266at_err_t err;
    int r = -1;

    do
    {
        err = esp8266at_cmd_at_cwqap(esp8266at, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK) {
            break;
        }

        r = 0;

        break;
    } while(1);

    printf("result : err = %d\n", err);

    return r;
}

int esp8266at_cli_at_ap_query_ip(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = 0;
    esp8266at_err_t err;

    err = esp8266at_cmd_at_cifsr(esp8266at, _timeoutms, NULL);
    if (err != ESP8266AT_ERR_OK)
    {
        r = -1;
    }
    printf("result : err = %d, ip_addr = %s, mac_addr = %s\n", err, esp8266at->ip_addr, esp8266at->mac_addr);

    return r;
}

int esp8266at_cli_at_conn(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    char *tmpstr;
    int tmplen;
    char *cmd = NULL;
    int cmdlen = 0;

    tmpstr = str;
    tmplen = len;

    do
    {
        cmd = "open ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_conn_open(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "close";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_conn_close(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "send ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_conn_send(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "recv";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_conn_recv(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_conn_open(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    char type[64];
    char ip[128];
    uint32_t port;

    do
    {
        sscanf(str, "%s %s %lu", type, ip, &port);
        err = esp8266at_cmd_at_cipstart(esp8266at, type, ip, port, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_conn_close(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    esp8266at_err_t err;

    do
    {
        err = esp8266at_cmd_at_cipclose(esp8266at, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_conn_send(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    esp8266at_err_t err;

    do
    {
        err = esp8266at_cmd_at_cipsend(esp8266at, (uint8_t*) str, (uint32_t) len, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;

}

int esp8266at_cli_at_conn_recv(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    uint32_t read_len;
    uint32_t read = 0;

    do
    {
        sscanf(str, "%lu", &read_len);
        err = esp8266at_cmd_at_ciprecv(esp8266at, _recv_buf, read_len, &read, _timeoutms, NULL);
        _recv_buf[read] = 0;

        break;
    } while (1);


    printf("\"%s\"\n", (char*) _recv_buf);

    printf("result : err = %d\n", err);

    r = 0;

    return r;
}

int esp8266at_cli_at_mqtt(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    char *tmpstr;
    int tmplen;
    char *cmd = NULL;
    int cmdlen = 0;

    tmpstr = str;
    tmplen = len;

    do
    {

#if (ESP8266AT__USE_WIZFI360_API == 1)
        cmd = "topic ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_topic(esp8266at, tmpstr, tmplen, arg);
            break;
        }
#else
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

        cmd = "open ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_open(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "close";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_close(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "pub ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_pub(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "sub ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_sub(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "unsub ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_unsub(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "sublist";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_sublist(esp8266at, tmpstr, tmplen, arg);
            break;
        }


        cmd = "subget ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_mqtt_subget(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        break;
    } while (1);

    return r;
}

#if (ESP8266AT__USE_WIZFI360_API == 1)
int esp8266at_cli_at_mqtt_topic(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    char pub_topic[128];
    char sub_topic[128];
    char sub_topic_2[128];
    char sub_topic_3[128];

    memset(pub_topic, 0, sizeof(pub_topic));
    memset(sub_topic, 0, sizeof(sub_topic));
    memset(sub_topic_2, 0, sizeof(sub_topic_2));
    memset(sub_topic_3, 0, sizeof(sub_topic_3));

    do
    {
        sscanf(str, "%s %s %s %s", pub_topic, sub_topic, sub_topic_2, sub_topic_3);
        err = esp8266at_cmd_at_mqtttopic(esp8266at, pub_topic, sub_topic, sub_topic_2, sub_topic_3, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}
#else
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

int esp8266at_cli_at_mqtt_open(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    char ip[128];
    uint32_t port;
    uint32_t reconnect;

    do
    {
        sscanf(str, "%s %lu %lu", ip, &port, &reconnect);
        err = esp8266at_cmd_at_mqttconn(esp8266at, ip, port, reconnect, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_mqtt_close(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    esp8266at_err_t err;

    do
    {
        err = esp8266at_cmd_at_mqttclean(esp8266at, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_mqtt_pub(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    esp8266at_err_t err;
    char topic[128];
    uint32_t qos = 0;
    uint32_t retain = 0;

    do
    {
#if (ESP8266AT__USE_WIZFI360_API == 1)
        sscanf(str, "%s", _mqtt_msg_buf);
        err = esp8266at_cmd_at_mqttpub(esp8266at, topic, (char *) _mqtt_msg_buf, qos, retain, _timeoutms, NULL);
#else
        sscanf(str, "%s %s %lu %lu", topic, _mqtt_msg_buf, &qos, &retain);
        err = esp8266at_cmd_at_mqttpubraw(esp8266at, topic, (char *) _mqtt_msg_buf, strlen((char *)_mqtt_msg_buf), qos, retain, _timeoutms, NULL);
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_mqtt_sub(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    esp8266at_err_t err;
    char topic[128];
    uint32_t id;
    uint32_t qos;

    do
    {
        sscanf(str, "%lu %s %lu", &id, topic, &qos);
        err = esp8266at_cmd_at_mqttsub(esp8266at, id, topic, qos, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_mqtt_unsub(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    esp8266at_err_t err;
    uint32_t id;

    do
    {
        sscanf(str, "%lu", &id);
        err = esp8266at_cmd_at_mqttunsub(esp8266at, id, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_mqtt_sublist(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;
    esp8266at_err_t err;
    char topic[128];
    uint32_t qos;

    do
    {
        sscanf(str, "%s %lu", topic, &qos);
        err = esp8266at_cmd_at_mqttsub_q(esp8266at, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

int esp8266at_cli_at_mqtt_subget(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    uint32_t max_len;
    uint32_t read = 0;
    uint32_t id = 0;

    do
    {
        sscanf(str, "%lu %lu", &id, &max_len);
        err = esp8266at_cmd_at_mqttsubget(esp8266at, id, _recv_buf, max_len, &read, _timeoutms, NULL);
        _recv_buf[read] = 0;

        break;
    } while (1);

    printf("\"%s\"\n", (char*) _recv_buf);

    printf("result : err = %d\n", err);

    r = 0;

    return r;
}

int esp8266at_cli_rdate(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r;
    esp8266at_err_t err;

    struct timeval tv;
    struct tm tm_data;

    r = -1;
    do
    {
        err = esp8266at_cmd_at_cipsntptime(esp8266at, &tm_data, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        if (tm_data.tm_year <= 70)
        {
            err = ESP8266AT_ERR_ERROR;
            break;
        }

        tv.tv_sec = mktime(&tm_data);
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);

        r = 0;
        break;
    } while(1);

    printf("result : err = %d\n", err);

    return r;
}

int esp8266at_cli_echo_client(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;

    char ssid[64];
    char passwd[64];
    char ip[128];
    uint32_t port;
    uint32_t count = 0;
    char msg[256];
    uint32_t msglen;
    uint32_t read;

    do
    {
        sscanf(str, "%s %s %s %lu %lu", ssid, passwd, ip, &port, &count);

        printf("\n==== Quit from the AP ====\n\n");
        esp8266at_cmd_at_cwqap(esp8266at, _timeoutms, NULL);

        printf("\n==== Reset module ====\n\n");
        esp8266at_reset(esp8266at);

        printf("\n==== Config echo on ====\n\n");
        err = esp8266at_cmd_at_e(esp8266at, 1, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        printf("\n==== Config IP multiple connection mode 0 ====\n\n");
        err = esp8266at_cmd_at_cipmux(esp8266at, 0, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        printf("\n==== Config WiFi mode 1 ====\n\n");
        err = esp8266at_cmd_at_cwmode(esp8266at, 1, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        printf("\n==== Join to an AP ====\n\n");
        err = esp8266at_cmd_at_cwjap(esp8266at, ssid, passwd, _timeoutms * 3, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        printf("\n==== Query local IP ====\n\n");
        err = esp8266at_cmd_at_cifsr(esp8266at, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        printf("\n==== Open connection ====\n\n");
        err = esp8266at_cmd_at_cipstart(esp8266at, "TCP", ip, port, _timeoutms * 3, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        for (uint32_t i = 0; i < count; i++)
        {
            printf("\n---- Send message ----\n");
            sprintf(msg, "%03lu hello", i);
            msglen = strlen(msg);
            err = esp8266at_cmd_at_cipsend(esp8266at, (uint8_t*) msg, msglen, _timeoutms, NULL);
            if (err != ESP8266AT_ERR_OK)
            {
                break;
            }

            printf("\n---- Receive message ----\n");
            err = esp8266at_cmd_at_ciprecv(esp8266at, _recv_buf, msglen, &read, _timeoutms, NULL);
            if (err != ESP8266AT_ERR_OK)
            {
                break;
            }
            _recv_buf[read] = 0;

            if (memcmp(msg, _recv_buf, msglen) != 0)
            {
                err = ESP8266AT_ERR_ERROR;
                break;
            }

            printf("\"%s\"\n", (char*) _recv_buf);
        }
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        printf("\n==== Close connection ====\n\n");
        err = esp8266at_cmd_at_cipclose(esp8266at, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

        printf("\n==== Quit from the AP ====\n\n");
        err = esp8266at_cmd_at_cwqap(esp8266at, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }
        r = 0;

        break;
    } while (1);

    if (err != ESP8266AT_ERR_OK)
    {
        read = 0;
        esp8266at_io_read_timedms(esp8266at, _recv_buf, ESP8266AT_RECV_BUFFER_SIZE, &read, _timeoutms, NULL);
        _recv_buf[read] = 0;
        printf("\"%s\"\n", (char*) _recv_buf);
    }

    printf("result : err = %d\n", err);

    return r;
}

#endif /* (INCLUDE__ESP8266AT == 1) */

