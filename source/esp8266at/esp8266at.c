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

#undef LOGM_CATEGORY
#define LOGM_CATEGORY LOGM_CATEGORY__USER00

static esp8266at_err_t _wait_rsp(esp8266at_t *esp8266at, char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms, uint32_t* remain_timeoutms);
static esp8266at_err_t _send_cmd_and_wait_rsp(esp8266at_t *esp8266at, char *cmd, char *rsp, uint32_t timeoutms, uint32_t* remain_timeoutms);

esp8266at_err_t esp8266at_init(esp8266at_t *esp8266at) {
	int r;
	esp8266at_err_t err;

	assert(esp8266at->cmd_mutex == NULL);

	err = ESP8266AT_ERR_ERROR;

	do {
		r = mutex_create(&esp8266at->cmd_mutex);
		if (r != 0) {
			err = ESP8266AT_ERR_ERROR;
			break;
		}

		err = esp8266at_io_init(esp8266at);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		err = ESP8266AT_ERR_OK;

		break;
	} while (1);

	if (err != ESP8266AT_ERR_OK) {
		if (esp8266at->cmd_mutex != NULL) {
			mutex_delete(&esp8266at->cmd_mutex);
		}
	}

	return err;
}

esp8266at_err_t esp8266at_deinit(esp8266at_t *esp8266at) {
	esp8266at_err_t err;

	assert(esp8266at->cmd_mutex != NULL);

	err = esp8266at_io_deinit(esp8266at);

	mutex_delete(&esp8266at->cmd_mutex);

	return err;
}

static esp8266at_err_t _wait_rsp(esp8266at_t *esp8266at, char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	esp8266at_err_t err;
	uint32_t read;
	uint32_t rsp_len;
	uint32_t rsp_i;
	uint32_t buf_i;

	err = ESP8266AT_ERR_ERROR;

	logmd( "wait response : begin");
	logmfd("wait response : \"%s\"", rsp);

	do {

		if (received) {
			*received = 0;
		}

		rsp_len = strlen(rsp);
		rsp_i = 0;
		buf_i = 0;
		while ((rsp_i < rsp_len) && (buf_i < length - 1)) {
			err = esp8266at_io_read_timedms(esp8266at, &buffer[buf_i], 1, &read, timeoutms, &timeoutms);
			if (err != ESP8266AT_ERR_OK) {
				break;
			}
			if (read != 1) {
				err = ESP8266AT_ERR_ERROR;
				break;
			}

			if (rsp[rsp_i] == buffer[buf_i]) {
				rsp_i++;
			} else {
				rsp_i = 0;
			}

			buf_i++;
		}
		buffer[buf_i] = 0;

		if (rsp_i != rsp_len) {
			err = ESP8266AT_ERR_ERROR;
			break;
		}

		if (received) {
			*received = buf_i;
		}

		err = ESP8266AT_ERR_OK;

		break;
	} while (1);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	logmfd("wait response : err = %d, size = %d, data = \"%s\"", err, buf_i, buffer);
	logmd( "wait response : end");

	return err;
}

static esp8266at_err_t _send_cmd_and_wait_rsp(esp8266at_t *esp8266at, char *cmd, char *rsp, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	esp8266at_err_t err;

	err = ESP8266AT_ERR_ERROR;

	logmd( "send command : begin");
	logmfd("send command : command = \"%s\", expected response = \"%s\"", cmd, rsp);

	do {
		err = esp8266at_io_read_clear_timedms(esp8266at, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		err = esp8266at_io_write_timedms(esp8266at, (uint8_t*) cmd, strlen(cmd), NULL, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		err = _wait_rsp(esp8266at, rsp, esp8266at->rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, NULL, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		break;
	} while (1);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	logmfd("send command : err = %d", err);
	logmd( "send command : end");

	return err;
}

esp8266at_err_t esp8266at_cmd_at_test(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp(esp8266at, "AT\r\n", "OK\r\n", timeoutms, &timeoutms);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_rst(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp(esp8266at, "AT+RST\r\n", "OK\r\n", timeoutms, &timeoutms);

	task_sleepms(500);
	if (timeoutms < 500) {
		timeoutms = 0;
	} else {
		timeoutms -= 500;
	}

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_gmr(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	int size = 0;
	char *key = "AT version:";

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp(esp8266at, "AT+GMR\r\n", "OK\r\n", timeoutms, &timeoutms);

	if (err == ESP8266AT_ERR_OK) {
		do {
			ptr1 = strstr((char *) esp8266at->rsp_buf, key);
			if (ptr1 == NULL) {
				break;
			}
			ptr1 = (char *) (((unsigned int) ptr1) + strlen(key));

			ptr2 = strstr(ptr1, "(");
			if (ptr2 == NULL || ptr1 >= ptr2) {
				break;
			}

			size = (unsigned int) ptr2 - (unsigned int) ptr1;
			size = min(size, ESP8266AT_VERSION_LENGTH_MAX);
			strncpy(esp8266at->version, ptr1, size);

			break;
		} while(1);
	}

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_e(esp8266at_t *esp8266at, int is_on, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	sprintf(esp8266at->cmd_buf, "ATE%d\r\n", is_on);
	err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->cmd_buf, "OK\r\n", timeoutms, &timeoutms);

	task_sleepms(100);
	if (timeoutms < 100) {
		timeoutms = 0;
	} else {
		timeoutms -= 100;
	}

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cwmode(esp8266at_t *esp8266at, int mode, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	sprintf(esp8266at->cmd_buf, "AT+CWMODE=%d\r\n", mode);
	err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->cmd_buf, "OK\r\n", timeoutms, &timeoutms);

	task_sleepms(100);
	if (timeoutms < 100) {
		timeoutms = 0;
	} else {
		timeoutms -= 100;
	}

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipmux(esp8266at_t *esp8266at, int mode, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	sprintf(esp8266at->cmd_buf, "AT+CIPMUX=%d\r\n", mode);
	err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->cmd_buf, "OK\r\n", timeoutms, &timeoutms);

	task_sleepms(100);
	if (timeoutms < 100) {
		timeoutms = 0;
	} else {
		timeoutms -= 100;
	}

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cwjap(esp8266at_t *esp8266at, char *ssid, char *passwd, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	sprintf(esp8266at->cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
	err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->cmd_buf, "OK\r\n", timeoutms, &timeoutms);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cwqap(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	task_sleepms(500);
	if (timeoutms < 500) {
		timeoutms = 0;
	} else {
		timeoutms -= 500;
	}

	err = _send_cmd_and_wait_rsp(esp8266at, "AT+CWQAP\r\n", "OK\r\n", timeoutms, &timeoutms);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cifsr(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	int size = 0;
	char *key = "STAIP,\"";
	char *key2 = "STAMAC,\"";

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp(esp8266at, "AT+CIFSR\r\n", "OK\r\n", timeoutms, &timeoutms);

	if (err == ESP8266AT_ERR_OK) {
		do {
			ptr1 = strstr((char *) esp8266at->rsp_buf, key);
			if (ptr1 == NULL) {
				break;
			}
			ptr1 = (char *) (((unsigned int) ptr1) + strlen(key));

			ptr2 = strstr(ptr1, "\"");
			if (ptr2 == NULL || ptr1 >= ptr2) {
				break;
			}

			size = (unsigned int) ptr2 - (unsigned int) ptr1;
			size = min(size, ESP8266AT_IP_ADDR_LENGTH_MAX);
			strncpy(esp8266at->ip_addr, ptr1, size);

			break;
		} while(1);

		do {
			ptr1 = strstr((char *) esp8266at->rsp_buf, key2);
			if (ptr1 == NULL) {
				break;
			}
			ptr1 = (char *) (((unsigned int) ptr1) + strlen(key2));

			ptr2 = strstr(ptr1, "\"");
			if (ptr2 == NULL || ptr1 >= ptr2) {
				break;
			}

			size = (unsigned int) ptr2 - (unsigned int) ptr1;
			size = min(size, ESP8266AT_MAC_ADDR_LENGTH_MAX);
			strncpy(esp8266at->mac_addr, ptr1, size);

			break;
		} while(1);
	}

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipstart(esp8266at_t *esp8266at, char *type, char *ip, uint32_t port, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	sprintf(esp8266at->cmd_buf, "AT+CIPSTART=\"%s\",\"%s\",%lu\r\n", type, ip, port);
	err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->cmd_buf, "OK\r\n", timeoutms, &timeoutms);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipstart_multiple(esp8266at_t *esp8266at, int id, char *type, char *ip, uint32_t port, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	sprintf(esp8266at->cmd_buf, "AT+CIPSTART=%d,\"%s\",\"%s\",%lu\r\n", id, type, ip, port);
	err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->cmd_buf, "OK\r\n", timeoutms, &timeoutms);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipclose(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	err = _send_cmd_and_wait_rsp(esp8266at, "AT+CIPCLOSE\r\n", "OK\r\n", timeoutms, &timeoutms);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_cipsend(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	err = ESP8266AT_ERR_ERROR;

	do {
		sprintf(esp8266at->cmd_buf, "AT+CIPSEND=%lu\r\n", length);
		err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->cmd_buf, "OK\r\n>", timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		err = esp8266at_io_write_timedms(esp8266at, buffer, length, NULL, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		err = _wait_rsp(esp8266at, "SEND OK\r\n", esp8266at->rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, NULL, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		break;
	} while (1);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

esp8266at_err_t esp8266at_cmd_at_ciprecv(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;
	uint32_t read;
	uint32_t data_len;

	r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
	timeoutms = task_getremainingtimeoutms();
	if (r == UBIK_ERR__TIMEOUT) {
		return ESP8266AT_ERR_TIMEOUT;
	}

	err = ESP8266AT_ERR_ERROR;

	do {
		if (received) {
			*received = 0;
		}

		err = _wait_rsp(esp8266at, "+IPD,", esp8266at->rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, NULL, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		err = _wait_rsp(esp8266at, ":", esp8266at->rsp_buf, ESP8266AT_RSP_BUFFER_SIZE, &read, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK) {
			break;
		}

		esp8266at->rsp_buf[read] = 0;
		data_len = atoi((char*) esp8266at->rsp_buf);

		err = esp8266at_io_read_timedms(esp8266at, buffer, data_len, &read, timeoutms, &timeoutms);
		if (err != ESP8266AT_ERR_OK && err != ESP8266AT_ERR_TIMEOUT) {
			break;
		}

		if (received) {
			*received = read;
		}

		err = ESP8266AT_ERR_OK;

		break;
	} while (1);

	if (remain_timeoutms) {
		* remain_timeoutms = timeoutms;
	}

	mutex_unlock(esp8266at->cmd_mutex);

	return err;
}

