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
#include <esp8266at.h>
#include <esp8266at_cli.h>

#if (INCLUDE__ESP8266AT == 1)

#include "../../source/esp8266at/esp8266at_io.h"

#undef LOGM_CATEGORY
#define LOGM_CATEGORY LOGM_CATEGORY__USER00

#define ESP8266AT_RECV_BUFFER_SIZE 1500

static uint8_t _recv_buf[ESP8266AT_RECV_BUFFER_SIZE];

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

        cmd = "restart";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_restart(esp8266at);
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

        cmd = "c ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_config(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "ap join ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at_ap_join(esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "ap quit";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_ap_quit(esp8266at);
            r = 0;
            break;
        }

        cmd = "ip";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            esp8266at_cli_at_query_ip(esp8266at);
            r = 0;
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

void esp8266at_cli_at_restart(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_rst(esp8266at, _timeoutms, NULL);
    printf("result : err = %d\n", err);
}

void esp8266at_cli_at_query_version(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_gmr(esp8266at, _timeoutms, NULL);
    printf("result : err = %d, version = %s\n", err, esp8266at->version);
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

int esp8266at_cli_at_ap_join(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;
    char ssid[128];
    char passwd[128];

    do
    {
        sscanf(str, "%s %s", ssid, passwd);
        err = esp8266at_cmd_at_cwjap(esp8266at, ssid, passwd, _timeoutms, NULL);
        printf("result : err = %d\n", err);
        r = 0;

        break;
    } while (1);

    return r;
}

void esp8266at_cli_at_ap_quit(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_cwqap(esp8266at, _timeoutms, NULL);
    printf("result : err = %d\n", err);
}

void esp8266at_cli_at_query_ip(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_cifsr(esp8266at, _timeoutms, NULL);
    printf("result : err = %d, ip_addr = %s, mac_addr = %s\n", err, esp8266at->ip_addr, esp8266at->mac_addr);
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
            esp8266at_cli_at_conn_close(esp8266at);
            r = 0;
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

            esp8266at_cli_at_conn_recv(esp8266at, tmpstr, tmplen, arg);
            r = 0;
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

void esp8266at_cli_at_conn_close(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_cipclose(esp8266at, _timeoutms, NULL);
    printf("result : err = %d\n", err);
}

int esp8266at_cli_at_conn_send(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    esp8266at_err_t err;

    err = esp8266at_cmd_at_cipsend(esp8266at, (uint8_t*) str, (uint32_t) len, _timeoutms, NULL);
    printf("result : err = %d\n", err);

    return 0;
}

void esp8266at_cli_at_conn_recv(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
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
}


int esp8266at_cli_echo_client(esp8266at_t *esp8266at, char *str, int len, void *arg)
{
    int r = -1;

    esp8266at_err_t err;

    char ssid[128];
    char passwd[128];
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

        printf("\n==== Restarts a module ====\n\n");
        err = esp8266at_cmd_at_rst(esp8266at, _timeoutms, NULL);
        if (err != ESP8266AT_ERR_OK)
        {
            r = -1;
            break;
        }

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
        err = esp8266at_cmd_at_cipstart(esp8266at, "TCP", ip, port, _timeoutms, NULL);
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

