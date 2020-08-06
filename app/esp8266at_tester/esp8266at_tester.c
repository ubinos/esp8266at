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

#define ESP8266AT_RECV_BUFFER_SIZE 1500

static uint8_t _recv_buf[ESP8266AT_RECV_BUFFER_SIZE];

static uint32_t _timeoutms = 10000;

static void rootfunc(void *arg);

static int clihookfunc(char *str, int len, void *arg);
static void clihelphookfunc();

static int cli_at(char *str, int len, void *arg);

static void cli_at_test(void);
static void cli_at_restart(void);
static void cli_at_query_version(void);
static void cli_at_query_ip(void);

static int cli_at_config(char *str, int len, void *arg);
static int cli_at_config_echo(char *str, int len, void *arg);
static int cli_at_config_wmode(char *str, int len, void *arg);
static int cli_at_config_ipmux(char *str, int len, void *arg);

static int cli_at_ap_join(char *str, int len, void *arg);
static void cli_at_ap_quit(void);

static int cli_at_conn(char *str, int len, void *arg);
static int cli_at_conn_open(char *str, int len, void *arg);
static void cli_at_conn_close(void);
static int cli_at_conn_send(char *str, int len, void *arg);
static void cli_at_conn_recv(void);

static int cli_echo_client(char *str, int len, void *arg);

static void cli_echo_client2(void);

int appmain(int argc, char *argv[]) {
	int r;

	r = task_create(NULL, rootfunc, NULL, task_getmiddlepriority(), 0, "root");
	if (0 != r) {
		logme("fail at task_create\r\n");
	}

	ubik_comp_start();

	return 0;
}

static void rootfunc(void *arg) {
	int r;

	HAL_Init();

	esp8266at_init();

	printf("\n\n\r\n");
	printf("================================================================================\r\n");
	printf("esp8266at_tester (build time: %s %s)\r\n", __TIME__, __DATE__);
	printf("================================================================================\r\n");
	printf("\r\n");

	dtty_setecho(1);

	r = cli_sethookfunc(clihookfunc, NULL);
	if (0 != r) {
		logme("fail at cli_sethookfunc\r\n");
	}

	r = cli_sethelphookfunc(clihelphookfunc);
	if (0 != r) {
		logme("fail at cli_sethelphookfunc\r\n");
	}

	r = cli_setprompt("esp8266at_tester> ");
	if (0 != r) {
		logme("fail at cli_setprompt\r\n");
	}

	r = task_create(NULL, cli_main, NULL, task_getmiddlepriority(), 192, "cli_main");
	if (0 != r) {
		logme("fail at task_create\r\n");
	}
}

static int clihookfunc(char *str, int len, void *arg) {
	int r = -1;
	char *tmpstr;
	int tmplen;
	char *cmd = NULL;
	int cmdlen = 0;

	tmpstr = str;
	tmplen = len;

	do {
		cmd = "at ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at(tmpstr, tmplen, arg);
			break;
		}

		cmd = "echo client ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_echo_client(tmpstr, tmplen, arg);
			break;
		}

		cmd = "echo client2";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			cli_echo_client2();
			r = 0;
			break;
		}
	} while (0);

	return r;
}

static void clihelphookfunc() {
	printf("\r\n");
	printf("at test                          : Tests AT startup\r\n");
	printf("at restart                       : Restarts a module\r\n");
	printf("at version                       : Query version information\r\n");
	printf("at config echo <on|off>          : Config echo\r\n");
	printf("at config wmode <mode>           : Config WiFi mode\r\n");
	printf("at config ipmux <mode>           : Config IP multiple connection mode\r\n");
	printf("at ap join <ssid> <passwd>       : Join to an AP\r\n");
	printf("at ap quit                       : Quit from the AP\r\n");
	printf("at ip                            : Query local IP\r\n");
	printf("at conn open <type> <ip> <port>  : Open connection\r\n");
	printf("at conn close                    : Close connection\r\n");
	printf("at conn send <data>              : Send data\r\n");
	printf("at conn recv                     : Receive data\r\n");
	printf("\r\n");
	printf("echo client <ssid> <passwd> <ip> <port> <count> : echo client test\r\n");
	printf("\r\n");
	printf("echo client2\r\n");
}

static int cli_at(char *str, int len, void *arg) {
	int r = -1;
	char *tmpstr;
	int tmplen;
	char *cmd = NULL;
	int cmdlen = 0;

	tmpstr = str;
	tmplen = len;

	do {
		cmd = "test";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			cli_at_test();
			r = 0;
			break;
		}

		cmd = "restart";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			cli_at_restart();
			r = 0;
			break;
		}

		cmd = "version";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			cli_at_query_version();
			r = 0;
			break;
		}

		cmd = "config ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_config(tmpstr, tmplen, arg);
			break;
		}

		cmd = "ap join ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_ap_join(tmpstr, tmplen, arg);
			break;
		}

		cmd = "ap quit";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			cli_at_ap_quit();
			r = 0;
			break;
		}

		cmd = "ip";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			cli_at_query_ip();
			r = 0;
			break;
		}

		cmd = "conn ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_conn(tmpstr, tmplen, arg);
			break;
		}
	} while (0);

	return r;
}

static void cli_at_test() {
	esp8266at_err_t err;

	err = esp8266at_cmd_at_test(_timeoutms);
	printf("result : %d\r\n", err);
}

static void cli_at_restart() {
	esp8266at_err_t err;

	err = esp8266at_cmd_at_rst(_timeoutms);
	printf("result : %d\r\n", err);
}

static void cli_at_query_version() {
	esp8266at_err_t err;

	err = esp8266at_cmd_at_gmr(_timeoutms);
	printf("result : %d\r\n", err);
}

static int cli_at_config(char *str, int len, void *arg) {
	int r = -1;
	char *tmpstr;
	int tmplen;
	char *cmd = NULL;
	int cmdlen = 0;

	tmpstr = str;
	tmplen = len;

	do {
		cmd = "echo ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_config_echo(tmpstr, tmplen, arg);
			break;
		}

		cmd = "wmode ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_config_wmode(tmpstr, tmplen, arg);
			break;
		}

		cmd = "ipmux ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_config_ipmux(tmpstr, tmplen, arg);
			break;
		}
	} while (0);

	return r;
}

static int cli_at_config_echo(char *str, int len, void *arg) {
	int r = -1;
	char *tmpstr;
	int tmplen;
	char *cmd = NULL;
	int cmdlen = 0;

	esp8266at_err_t err;
	int is_on;

	tmpstr = str;
	tmplen = len;

	do {
		cmd = "on";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			is_on = 1;
		} else {
			cmd = "off";
			cmdlen = strlen(cmd);
			if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
				is_on = 0;
			} else {
				r = -1;
				break;
			}
		}

		err = esp8266at_cmd_at_e(is_on, _timeoutms);
		printf("result : %d\r\n", err);
		r = 0;
	} while (0);

	return r;
}

static int cli_at_config_wmode(char *str, int len, void *arg) {
	int r = -1;

	esp8266at_err_t err;
	int mode;

	mode = atoi(str);

	err = esp8266at_cmd_at_cwmode(mode, _timeoutms);
	printf("result : %d\r\n", err);
	r = 0;

	return r;
}

static int cli_at_config_ipmux(char *str, int len, void *arg) {
	int r = -1;

	esp8266at_err_t err;
	int mode;

	mode = atoi(str);

	err = esp8266at_cmd_at_cipmux(mode, _timeoutms);
	printf("result : %d\r\n", err);
	r = 0;

	return r;
}

static int cli_at_ap_join(char *str, int len, void *arg) {
	int r = -1;

	esp8266at_err_t err;
	char ssid[128];
	char passwd[128];

	do {
		sscanf(str, "%s %s", ssid, passwd);
		err = esp8266at_cmd_at_cwjap(ssid, passwd, _timeoutms);
		printf("result : %d\r\n", err);
		r = 0;
	} while (0);

	return r;
}

static void cli_at_ap_quit(void) {
	esp8266at_err_t err;

	err = esp8266at_cmd_at_cwqap(_timeoutms);
	printf("result : %d\r\n", err);
}

static void cli_at_query_ip(void) {
	esp8266at_err_t err;

	err = esp8266at_cmd_at_cifsr(_timeoutms);
	printf("result : %d\r\n", err);
}

static int cli_at_conn(char *str, int len, void *arg) {
	int r = -1;
	char *tmpstr;
	int tmplen;
	char *cmd = NULL;
	int cmdlen = 0;

	tmpstr = str;
	tmplen = len;

	do {
		cmd = "open ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_conn_open(tmpstr, tmplen, arg);
			break;
		}

		cmd = "close";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			cli_at_conn_close();
			r = 0;
			break;
		}

		cmd = "send ";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			tmpstr = &tmpstr[cmdlen];
			tmplen -= cmdlen;

			r = cli_at_conn_send(tmpstr, tmplen, arg);
			break;
		}

		cmd = "recv";
		cmdlen = strlen(cmd);
		if (tmplen >= cmdlen && strncmp(tmpstr, cmd, cmdlen) == 0) {
			cli_at_conn_recv();
			r = 0;
			break;
		}
	} while (0);

	return r;
}

static int cli_at_conn_open(char *str, int len, void *arg) {
	int r = -1;

	esp8266at_err_t err;
	char type[64];
	char ip[128];
	uint32_t port;

	do {
		sscanf(str, "%s %s %lu", type, ip, &port);
		err = esp8266at_cmd_at_cipstart(type, ip, port, _timeoutms);
		printf("result : %d\r\n", err);
		r = 0;
	} while (0);

	return r;
}

static void cli_at_conn_close(void) {
	esp8266at_err_t err;

	err = esp8266at_cmd_at_cipclose(_timeoutms);
	printf("result : %d\r\n", err);
}

static int cli_at_conn_send(char *str, int len, void *arg) {
	esp8266at_err_t err;

	err = esp8266at_cmd_at_cipsend((uint8_t*) str, (uint32_t) len, _timeoutms);
	printf("result : %d\r\n", err);

	return 0;
}

static void cli_at_conn_recv(void) {
	esp8266at_err_t err;
	uint32_t read = 0;

	err = esp8266at_cmd_at_ciprecv(_recv_buf, ESP8266AT_RECV_BUFFER_SIZE, &read, _timeoutms);
	_recv_buf[read] = 0;

	printf("\"%s\"\r\n", (char*) _recv_buf);

	printf("result : %d\r\n", err);
}

static void cli_echo_client2(void) {
	char *cmd = "ssid passwd 192.168.0.2 9000 1000";
	cli_echo_client(cmd, strlen(cmd), NULL);
}

static int cli_echo_client(char *str, int len, void *arg) {
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

	do {
		sscanf(str, "%s %s %s %lu %lu", ssid, passwd, ip, &port, &count);

		printf("\r\n==== Quit from the AP ====\r\n");
		err = esp8266at_cmd_at_cwqap(_timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Restarts a module ====\r\n");
		err = esp8266at_cmd_at_rst(_timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Config echo on ====\r\n");
		err = esp8266at_cmd_at_e(1, _timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Config IP multiple connection mode 0 ====\r\n");
		err = esp8266at_cmd_at_cipmux(0, _timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Config WiFi mode 1 ====\r\n");
		err = esp8266at_cmd_at_cwmode(1, _timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Join to an AP ====\r\n");
		err = esp8266at_cmd_at_cwjap(ssid, passwd, _timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Query local IP ====\r\n");
		err = esp8266at_cmd_at_cifsr(_timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Open connection ====\r\n");
		err = esp8266at_cmd_at_cipstart("TCP", ip, port, _timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		for (uint32_t i = 0; i < count; i++) {
			printf("\r\n---- Send message ----\r\n");
			sprintf(msg, "%03lu hello", i);
			msglen = strlen(msg);
			err = esp8266at_cmd_at_cipsend((uint8_t *) msg, msglen, _timeoutms);
			if (err != ESP8266AT_OK) {
				break;
			}

			printf("\r\n---- Receive message ----\r\n");
			err = esp8266at_cmd_at_ciprecv(_recv_buf, ESP8266AT_RECV_BUFFER_SIZE, &read, _timeoutms);
			if (err != ESP8266AT_OK) {
				break;
			}
			_recv_buf[read] = 0;

			if (memcmp(msg, _recv_buf, msglen) != 0) {
				err = ESP8266AT_ERROR;
				break;
			}

			printf("\"%s\"\r\n", (char*) _recv_buf);
		}
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Close connection ====\r\n");
		err = esp8266at_cmd_at_cipclose(_timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}

		printf("\r\n==== Quit from the AP ====\r\n");
		err = esp8266at_cmd_at_cwqap(_timeoutms);
		if (err != ESP8266AT_OK) {
			r = -1;
			break;
		}
		r = 0;
	} while (0);

	printf("result : %d\r\n", err);

	return r;
}

#endif /* (INCLUDE__APP__esp8266at_tester == 1) */

