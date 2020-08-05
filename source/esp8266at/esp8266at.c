/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp8266at.h>

#include "esp8266at_io.h"

#define ESP8266AT_CMD_BUFFER_SIZE 256
#define ESP8266AT_RSP_BUFFER_SIZE 256

#define ESP8266AT_DEBUG 1

#if (ESP8266AT_DEBUG == 1)
#define esp8266at_debugf(...) printf(__VA_ARGS__)
#else
	#define esp8266at_debugf(...)
#endif

static char _cmd_buf[ESP8266AT_CMD_BUFFER_SIZE];
static uint8_t _rsp_buf[ESP8266AT_RSP_BUFFER_SIZE];

static esp8266at_err_t _wait_rsp(char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms);
static esp8266at_err_t _send_cmd_and_wait_rsp(char *cmd, char *rsp, uint32_t timeoutms);

esp8266at_err_t esp8266at_init(void) {
	esp8266at_err_t err;

	err = esp8266at_io_init();

	return err;
}

esp8266at_err_t esp8266at_deinit(void) {
	esp8266at_err_t err;

	err = esp8266at_io_deinit();

	return err;
}

static esp8266at_err_t _wait_rsp(char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms)  {
	esp8266at_err_t err;
	uint32_t read;
	uint32_t rsp_len;
	uint32_t rsp_i;
	uint32_t buf_i;

	err = ESP8266AT_ERROR;

	do {
		esp8266at_debugf("receive : \"");

		if (received) {
			*received = 0;
		}

		rsp_len = strlen(rsp);
		rsp_i = 0;
		buf_i = 0;
		while ((rsp_i < rsp_len) && (buf_i < length)) {
			err = esp8266at_io_read_timedms(&buffer[buf_i], 1, &read, timeoutms);
			timeoutms = task_getremainingtimeout();
			if (err != ESP8266AT_OK) {
				break;
			}
			if (read != 1) {
				err = ESP8266AT_ERROR;
				break;
			}

			esp8266at_debugf("%c", buffer[buf_i]);

			if (rsp[rsp_i] == buffer[buf_i]) {
				rsp_i++;
			} else {
				rsp_i = 0;
			}

			buf_i++;
		}

		esp8266at_debugf("\"\r\n");

		if (rsp_i != rsp_len) {
			err = ESP8266AT_ERROR;
			break;
		}

		if (received) {
			*received = buf_i;
		}

		err = ESP8266AT_OK;
	} while (0);

	return err;
}

static esp8266at_err_t _send_cmd_and_wait_rsp(char *cmd, char *rsp, uint32_t timeoutms) {
	esp8266at_err_t err;

	err = ESP8266AT_ERROR;

	do {
		esp8266at_debugf("send    : \"%s\"\r\n", cmd);
		esp8266at_debugf("expect  : \"%s\"\r\n", rsp);

		err = esp8266at_io_read_clear_timedms(timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}

		err = esp8266at_io_write_timedms((uint8_t *) cmd, strlen(cmd), NULL, timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}

		err = _wait_rsp(rsp, _rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, NULL, timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}
	} while (0);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_test(uint32_t timeoutms) {
	return _send_cmd_and_wait_rsp("AT\r\n", "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_rst(uint32_t timeoutms) {
	return _send_cmd_and_wait_rsp("AT+RST\r\n", "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_gmr(uint32_t timeoutms) {
	return _send_cmd_and_wait_rsp("AT+GMR\r\n", "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_e(int is_on, uint32_t timeoutms) {
	sprintf(_cmd_buf, "ATE%d\r\n", is_on);
	return _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_cwmode(int mode, uint32_t timeoutms) {
	sprintf(_cmd_buf, "AT+CWMODE=%d\r\n", mode);
	return _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_cipmux(int mode, uint32_t timeoutms) {
	esp8266at_err_t err;

	sprintf(_cmd_buf, "AT+CIPMUX=%d\r\n", mode);
	err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);
	task_sleep(500);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cwjap(char *ssid, char *passwd, uint32_t timeoutms) {
	sprintf(_cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
	return _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_cwqap(uint32_t timeoutms) {
	esp8266at_err_t err;

	task_sleep(500);
	err = _send_cmd_and_wait_rsp("AT+CWQAP\r\n", "OK\r\n", timeoutms);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cifsr(uint32_t timeoutms) {
	return _send_cmd_and_wait_rsp("AT+CIFSR\r\n", "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_cipstart(char *type, char *ip, uint32_t port, uint32_t timeoutms) {
	sprintf(_cmd_buf, "AT+CIPSTART=\"%s\",\"%s\",%lu\r\n", type, ip, port);
	return _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_cipstart_multiple(int id, char *type, char *ip, uint32_t port, uint32_t timeoutms) {
	sprintf(_cmd_buf, "AT+CIPSTART=%d,\"%s\",\"%s\",%lu\r\n", id, type, ip, port);
	return _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_cipclose(uint32_t timeoutms) {
	return _send_cmd_and_wait_rsp("AT+CIPCLOSE\r\n", "OK\r\n", timeoutms);
}

esp8266at_err_t esp8266at_cmd_at_cipsend(uint8_t *buffer, uint32_t length, uint32_t timeoutms) {
	esp8266at_err_t err;

	err = ESP8266AT_ERROR;

	do {
		sprintf(_cmd_buf, "AT+CIPSEND=%lu\r\n", length);
		err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n>", timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}

		err = esp8266at_io_write_timedms(buffer, length, NULL, timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}

		err = _wait_rsp("SEND OK\r\n", _rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, NULL, timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}
	} while (0);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_ciprecv(uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms) {
	esp8266at_err_t err;
	uint32_t read;
	uint32_t data_len;

	err = ESP8266AT_ERROR;

	do {
		if (received) {
			*received = 0;
		}

		err = _wait_rsp("+IPD,", _rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, NULL, timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}

		err = _wait_rsp(":", _rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, &read, timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK) {
			break;
		}

		_rsp_buf[read] = 0;
		data_len = atoi((char *) _rsp_buf);

		err = esp8266at_io_read_timedms(buffer, data_len, &read, timeoutms);
		timeoutms = task_getremainingtimeout();
		if (err != ESP8266AT_OK && err != ESP8266AT_TIMEOUT) {
			break;
		}

		if (received) {
			*received = read;
		}

		err = ESP8266AT_OK;
	} while (0);

	return err;
}


