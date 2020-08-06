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

static mutex_pt _cmd_mutex = NULL;

static esp8266at_err_t _wait_rsp(char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms);
static esp8266at_err_t _send_cmd_and_wait_rsp(char *cmd, char *rsp, uint32_t timeoutms);

esp8266at_err_t esp8266at_init(void) {
	int r;
	esp8266at_err_t err;

	assert(_cmd_mutex == NULL);

	err = ESP8266AT_ERROR;

	do {
		r = mutex_create(&_cmd_mutex);
		if (r != 0) {
			err = ESP8266AT_ERROR;
			break;
		}

		err = esp8266at_io_init();
		if (err != ESP8266AT_OK) {
			break;
		}

		err = ESP8266AT_OK;
	} while (0);

	if (err != ESP8266AT_OK) {
		if (_cmd_mutex != NULL) {
			mutex_delete(&_cmd_mutex);
		}
	}

	return err;
}

esp8266at_err_t esp8266at_deinit(void) {
	esp8266at_err_t err;

	assert(_cmd_mutex != NULL);

	err = esp8266at_io_deinit();

	mutex_delete(&_cmd_mutex);

	return err;
}

static esp8266at_err_t _wait_rsp(char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms) {
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

		err = esp8266at_io_write_timedms((uint8_t*) cmd, strlen(cmd), NULL, timeoutms);
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
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp("AT\r\n", "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_rst(uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp("AT+RST\r\n", "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_gmr(uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp("AT+GMR\r\n", "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_e(int is_on, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	sprintf(_cmd_buf, "ATE%d\r\n", is_on);
	err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cwmode(int mode, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	sprintf(_cmd_buf, "AT+CWMODE=%d\r\n", mode);
	err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipmux(int mode, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	sprintf(_cmd_buf, "AT+CIPMUX=%d\r\n", mode);
	err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);

	task_sleep(500);
	if (timeoutms < 500) {
		timeoutms = 0;
	} else {
		timeoutms -= 500;
	}

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cwjap(char *ssid, char *passwd, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	sprintf(_cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
	err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cwqap(uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	task_sleep(500);
	if (timeoutms < 500) {
		timeoutms = 0;
	} else {
		timeoutms -= 500;
	}

	err = _send_cmd_and_wait_rsp("AT+CWQAP\r\n", "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cifsr(uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp("AT+CIFSR\r\n", "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipstart(char *type, char *ip, uint32_t port, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	sprintf(_cmd_buf, "AT+CIPSTART=\"%s\",\"%s\",%lu\r\n", type, ip, port);
	err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipstart_multiple(int id, char *type, char *ip, uint32_t port, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	sprintf(_cmd_buf, "AT+CIPSTART=%d,\"%s\",\"%s\",%lu\r\n", id, type, ip, port);
	err = _send_cmd_and_wait_rsp(_cmd_buf, "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipclose(uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp("AT+CIPCLOSE\r\n", "OK\r\n", timeoutms);

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipsend(uint8_t *buffer, uint32_t length, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

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

	mutex_unlock(_cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_ciprecv(uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;
	uint32_t read;
	uint32_t data_len;

	r = mutex_lock_timedms(_cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_TIMEOUT;
	}

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
		data_len = atoi((char*) _rsp_buf);

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

	mutex_unlock(_cmd_mutex);

	return err;
}

