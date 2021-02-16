/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>

#if (INCLUDE__APP__esp8266at_tester == 1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp8266at.h>
#include <esp8266at_cli.h>

#if (UBINOS__BSP__STM32_STM32XXXX == 1)

#include "main.h"

#endif /* (UBINOS__BSP__STM32_STM32XXXX == 1) */

static void rootfunc(void *arg);

static int clihookfunc(char *str, int len, void *arg);
static void clihelphookfunc();

static void esp8266at_cli_echo_client2(esp8266at_t *esp8266at);

int appmain(int argc, char *argv[])
{
    int r;

    r = task_create(NULL, rootfunc, NULL, task_getmiddlepriority(), 0, "root");
    if (0 != r)
    {
        logme("fail at task_create");
    }

    ubik_comp_start();

    return 0;
}

static void rootfunc(void *arg)
{
    int r;

#if (UBINOS__BSP__STM32_STM32XXXX == 1)
    HAL_Init();
#endif /* (UBINOS__BSP__STM32_STM32XXXX == 1) */

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
    printf("at test                          : Tests AT startup\n");
    printf("at restart                       : Restarts a module\n");
    printf("at version                       : Query version information\n");
    printf("at config echo <on|off>          : Config echo\n");
    printf("at config wmode <mode>           : Config WiFi mode\n");
    printf("at config ipmux <mode>           : Config IP multiple connection mode\n");
    printf("at ap join <ssid> <passwd>       : Join to an AP\n");
    printf("at ap quit                       : Quit from the AP\n");
    printf("at ip                            : Query local IP\n");
    printf("at conn open <type> <ip> <port>  : Open connection\n");
    printf("at conn close                    : Close connection\n");
    printf("at conn send <data>              : Send data\n");
    printf("at conn recv                     : Receive data\n");
    printf("\n");
    printf("echo client <ssid> <passwd> <ip> <port> <count> : echo client test\n");
    printf("\n");
    printf("echo client2\n");
}

static void esp8266at_cli_echo_client2(esp8266at_t *esp8266at)
{
    char *cmd = "ssid passwd 192.168.0.2 9000 1000";
    esp8266at_cli_echo_client(esp8266at, cmd, strlen(cmd), NULL);
}

#endif /* (INCLUDE__APP__esp8266at_tester == 1) */

