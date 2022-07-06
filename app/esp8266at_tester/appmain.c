/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp8266at.h>
#include <esp8266at_cli.h>

#include "main.h"

static void rootfunc(void *arg);

static int clihookfunc(char *str, int len, void *arg);
static void clihelphookfunc();

static void esp8266at_cli_echo_client2(esp8266at_t *esp8266at);

int appmain(int argc, char *argv[])
{
    int r;
    (void) r;

    r = task_create(NULL, rootfunc, NULL, task_getmiddlepriority(), 0, "root");
    ubi_assert(r == 0);

    ubik_comp_start();

    return 0;
}

static void rootfunc(void *arg)
{
    int r;
    (void) r;

#if (UBINOS__BSP__BOARD_VARIATION__STM32FOOTPAD == 1)
    power_init();
    wifi_enable();
#endif /* (UBINOS__BSP__BOARD_VARIATION__STM32FOOTPAD == 1) */

    esp8266at_init(&_g_esp8266at);

    printf("\n\n\n");
    printf("================================================================================\n");
    printf("esp8266at_tester (build time: %s %s)\n", __TIME__, __DATE__);
    printf("================================================================================\n");
    printf("\n");

    logm_setlevel(LOGM_CATEGORY__USER00, LOGM_LEVEL__DEBUG);

    r = cli_sethookfunc(clihookfunc, NULL);
    if (0 != r)
    {
        logme("fail at cli_sethookfunc");
    }

    r = cli_sethelphookfunc(clihelphookfunc);
    if (0 != r)
    {
        logme("fail at cli_sethelphookfunc");
    }

    r = cli_setprompt("esp8266at_tester> ");
    if (0 != r)
    {
        logme("fail at cli_setprompt");
    }

    r = task_create(NULL, cli_main, NULL, task_getmiddlepriority(), 192, "cli_main");
    if (0 != r)
    {
        logme("fail at task_create");
    }
}

static int clihookfunc(char *str, int len, void *arg)
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
        cmd = "at ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_at(&_g_esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "rdate";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            esp8266at_cli_rdate(&_g_esp8266at, tmpstr, tmplen, arg);
            r = 0;
            break;
        }

        cmd = "echo client ";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            r = esp8266at_cli_echo_client(&_g_esp8266at, tmpstr, tmplen, arg);
            break;
        }

        cmd = "echo client2";
        cmdlen = strlen(cmd);
        if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0)
        {
            tmpstr = &tmpstr[cmdlen];
            tmplen -= cmdlen;

            esp8266at_cli_echo_client2(&_g_esp8266at);
            r = 0;
            break;
        }

        break;
    } while (1);

    return r;
}

static void clihelphookfunc()
{
    printf("at i                                            : Interactive mode\n");
    printf("at test                                         : Tests AT startup\n");
    printf("at restart                                      : Restarts a module\n");
    printf("at version                                      : Query version information\n");
    printf("at dns                                          : Query DNS configuration\n");
    printf("at sntp                                         : Query SNTP configuration\n");
    printf("at time                                         : Query SNTP time\n");
    printf("\n");
    printf("at c echo <on|off>                              : Config echo\n");
    printf("at c wmode <mode>                               : Config WiFi mode\n");
    printf("    <mode> :\n");
    printf("        0 : Null mode. Wi-Fi RF will be disabled\n");
    printf("        1 : Station mode\n");
    printf("        2 : SoftAP mode\n");
    printf("        3 : SoftAP+Station mode\n");
    printf("at c ipmux <mode>                               : Config IP multiple connection mode\n");
    printf("    <mode> : connection mode (Default: 0)\n");
    printf("        0 : single connection\n");
    printf("        1 : multiple connections\n");
    printf("at c ap <ssid> <passwd>                         : Set AP join information\n");
    printf("at c dns <enalbe> (<server>)                    : Set DNS configuration\n");
    printf("    <enalbe> : enable\n");
    printf("        0 : disable\n");
    printf("        1 : enable\n");
    printf("    example: : at c dns 1 155.230.10.2 169.126.63.1\n");
    printf("at c sntp <enable> <tz> (<server>)              : Set SNTP configuration\n");
    printf("    <enalbe> : enable\n");
    printf("        0 : disable\n");
    printf("        1 : enable\n");
    printf("    <tz> : timezone (-12 to 14)\n");
    printf("    <server> : sntp server address\n");
    printf("    example: : at c sntp 1 9 time.google.com\n");
#if (ESP8266AT__USE_WIZFI360_API == 1)
    printf("at c mqtt <client_id> <username> <passwd>       : Set MQTT connection information\n");
#else
    printf("at c mqtt <mqtt_scheme> <client_id> <username> <passwd> : Set MQTT connection information\n");
    printf("    <mqtt_scheme> :\n");
    printf("        1 : MQTT over TCP\n");
    // printf("        2 : MQTT over TLS (no certificate verify)\n");
    // printf("        3 : MQTT over TLS (verify server certificate)\n");
    // printf("        4 : MQTT over TLS (provide client certificate)\n");
    // printf("        5 : MQTT over TLS (verify server certificate and provide client certificate)\n");
    // printf("        6 : MQTT over WebSocket (based on TCP)\n");
    // printf("        7 : MQTT over WebSocket Secure (based on TLS, no certificate verify)\n");
    // printf("        8 : MQTT over WebSocket Secure (based on TLS, verify server certificate)\n");
    // printf("        9 : MQTT over WebSocket Secure (based on TLS, provide client certificate)\n");
    // printf("        10: MQTT over WebSocket Secure (based on TLS, verify server certificate and provide client certificate)\n");
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */
    printf("\n");
    printf("at ap join                                      : Join to an AP\n");
    printf("at ap quit                                      : Quit from the AP\n");
    printf("at ap ip                                        : Query local IP\n");
    printf("\n");
    printf("at conn open <type> <ip> <port>     : Open connection\n");
    printf("    <type> : type of transmission (Default: TCP)\n");
    printf("        TCP   : TCP client\n");
    printf("at conn close                                   : Close connection\n");
    printf("at conn send <data>                             : Send data\n");
    printf("at conn recv <len>                              : Receive data\n");
    printf("\n");
#if (ESP8266AT__USE_WIZFI360_API == 1)
    printf("at mqtt topic <pub_topic> <sub_topic>( <sub_topic_2>( <sub_topic_3>))  : Set MQTT topics\n");
    printf("    mqtt message must contain header (+MQTTSUBRECV:0,\"<topic>\",<data_len>,<data>)");
    printf("at mqtt open <ip> <port>                        : Open MQTT connection\n");
    printf("at mqtt close                                   : Close MQTT connection\n");
    printf("at mqtt pub <data>                              : Publish MQTT messages\n");
#else
    printf("at mqtt open <ip> <port> <reconnect>            : Open MQTT connection\n");
    printf("    <reconnect> :\n");
    printf("        0 : MQTT will not reconnect automatically.\n");
    printf("        1 : MQTT will reconnect automatically. It takes more resources.\n");
    printf("at mqtt close                                   : Close MQTT connection\n");
    printf("at mqtt pub <topic> <data> <qos> <retain>       : Publish MQTT messages in string to a defined topic.\n");
    printf("    <qos> :\n");
    printf("        0 : Up to once\n");
    printf("        1 : At least once\n");
    printf("        2 : Exactly once\n");
    printf("    <retain> :\n");
    printf("        0 : Not retained\n");
    printf("        1 : Retained\n");
    printf("at mqtt sub <id> <topic> <qos>                  : Subscribe to defined MQTT topics with defined QoS.\n");
    printf("    <qos> :\n");
    printf("        0 : Up to once\n");
    printf("        1 : At least once\n");
    printf("        2 : Exactly once\n");
    printf("at mqtt unsub <id>                              : Unsubscribe\n");
    printf("at mqtt sublist                                 : List all MQTT topics that have been already subscribed\n");
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */
    printf("at mqtt subget <id> <max_len>                   : Get subscribed data\n");
    printf("\n");
    printf("rdate                                           : sync system time with NSTP time\n");
    printf("\n");
    printf("echo client <ssid> <passwd> <ip> <port> <count> : echo client test\n");
    printf("\n");
    printf("echo client2\n");
    printf("\n");
}

static void esp8266at_cli_echo_client2(esp8266at_t *esp8266at)
{
    char *cmd = "ssid passwd 192.168.0.2 9000 1000";
    esp8266at_cli_echo_client(esp8266at, cmd, strlen(cmd), NULL);
}

