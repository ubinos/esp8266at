/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>
#include <assert.h>

#include "../../../../esp8266at_io.h"

#include "main.h"

#undef LOGM_CATEGORY
#define LOGM_CATEGORY LOGM_CATEGORY__USER00

void esp8266at_io_callback(void);

extern esp8266at_t _esp8266at;
UART_HandleTypeDef _esp8266at_uart;

esp8266at_err_t esp8266at_io_init(esp8266at_t *esp8266at) {
	int r;
	esp8266at_err_t err;

	assert(esp8266at->io_read_sem == NULL);
	assert(esp8266at->io_mutex == NULL);

	err = ESP8266AT_ERROR;

	do {
		r = semb_create(&esp8266at->io_read_sem);
		if (r != 0) {
			err = ESP8266AT_ERROR;
			break;
		}

		r = mutex_create(&esp8266at->io_mutex);
		if (r != 0) {
			err = ESP8266AT_ERROR;
			break;
		}

		/* Set the WiFi USART configuration parameters */
		_esp8266at_uart.Instance = USARTx;
		_esp8266at_uart.Init.BaudRate = 115200;
		_esp8266at_uart.Init.WordLength = UART_WORDLENGTH_8B;
		_esp8266at_uart.Init.StopBits = UART_STOPBITS_1;
		_esp8266at_uart.Init.Parity = UART_PARITY_NONE;
		_esp8266at_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		_esp8266at_uart.Init.Mode = UART_MODE_TX_RX;
		_esp8266at_uart.Init.OverSampling = UART_OVERSAMPLING_16;

		/* Configure the USART IP */
		if (HAL_UART_Init(&_esp8266at_uart) != HAL_OK) {
			err = ESP8266AT_ERROR;
			break;
		}

		esp8266at->io_read_buffer.head = 0;
		esp8266at->io_read_buffer.tail = 0;

		HAL_UART_Receive_IT(&_esp8266at_uart, &esp8266at->io_read_buffer.data[0], 1);

		err = ESP8266AT_OK;

		break;
	} while (1);

	if (err != ESP8266AT_OK) {
		if (esp8266at->io_mutex != NULL) {
			mutex_delete(&esp8266at->io_mutex);
		}
		if (esp8266at->io_read_sem != NULL) {
			sem_delete(&esp8266at->io_read_sem);
		}
	}

	return err;
}

esp8266at_err_t esp8266at_io_deinit(esp8266at_t *esp8266at) {
	esp8266at_err_t err = ESP8266AT_OK;

	assert(esp8266at->io_read_sem != NULL);
	assert(esp8266at->io_mutex != NULL);

	if (HAL_UART_DeInit(&_esp8266at_uart) != HAL_OK) {
		err = ESP8266AT_ERROR;
	}

	mutex_delete(&esp8266at->io_mutex);
	sem_delete(&esp8266at->io_read_sem);

	return err;
}

esp8266at_err_t esp8266at_io_read_clear( esp8266at_t *esp8266at) {
	return esp8266at_io_read_clear_advan(esp8266at, 0, 0, NULL);
}

esp8266at_err_t esp8266at_io_read_clear_timedms(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	return esp8266at_io_read_clear_advan(esp8266at, ESP8266AT_IO_OPTION__TIMED, timeoutms, remain_timeoutms);
}

esp8266at_err_t esp8266at_io_read_clear_advan(esp8266at_t *esp8266at, uint16_t io_option, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		r = mutex_lock_timedms(esp8266at->io_mutex, timeoutms);
		if (r == UBIK_ERR__TIMEOUT) {
			return ESP8266AT_TIMEOUT;
		}
		timeoutms = task_getremainingtimeoutms();
	} else {
		r = mutex_lock(esp8266at->io_mutex);
	}
	if (r != 0) {
		return ESP8266AT_ERROR;
	}

	esp8266at->io_read_buffer.head = esp8266at->io_read_buffer.tail;

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		if (remain_timeoutms) {
			*remain_timeoutms = timeoutms;
		}
	}

	mutex_unlock(esp8266at->io_mutex);

	return ESP8266AT_OK;
}

esp8266at_err_t esp8266at_io_read(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read) {
	return esp8266at_io_read_advan(esp8266at, buffer, length, read, 0, 0, NULL);
}

esp8266at_err_t esp8266at_io_read_timedms(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	return esp8266at_io_read_advan(esp8266at, buffer, length, read, ESP8266AT_IO_OPTION__TIMED, timeoutms, remain_timeoutms);
}

esp8266at_err_t esp8266at_io_read_advan(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read, uint16_t io_option, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;
	uint32_t read_tmp;
	int is_first;

	assert(buffer != NULL);

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		r = mutex_lock_timedms(esp8266at->io_mutex, timeoutms);
		if (r == UBIK_ERR__TIMEOUT) {
			return ESP8266AT_TIMEOUT;
		}
		timeoutms = task_getremainingtimeoutms();
	} else {
		r = mutex_lock(esp8266at->io_mutex);
	}
	if (r != 0) {
		return ESP8266AT_ERROR;
	}

	read_tmp = 0;
	is_first = 1;
	err = ESP8266AT_IO_ERROR;

	while (read_tmp < length) {
		if (esp8266at->io_read_buffer.head != esp8266at->io_read_buffer.tail) {
			buffer[read_tmp] = esp8266at->io_read_buffer.data[esp8266at->io_read_buffer.head];
			esp8266at->io_read_buffer.head = (esp8266at->io_read_buffer.head + 1) % ESP8266AT_IO_READ_BUFFER_SIZE;

			read_tmp++;
		} else {
			if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
				if (is_first == 0 && timeoutms == 0) {
					err = ESP8266AT_TIMEOUT;
					break;
				}
				r = sem_take_timedms(esp8266at->io_read_sem, timeoutms);
				timeoutms = task_getremainingtimeoutms();
			} else {
				r = sem_take(esp8266at->io_read_sem);
			}
		}

		if (is_first) {
			is_first = 0;
		}
	}

	if (read != NULL) {
		*read = read_tmp;
	}

	if (read_tmp == length) {
		err = ESP8266AT_OK;
	}

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		if (remain_timeoutms) {
			*remain_timeoutms = timeoutms;
		}
	}

	mutex_unlock(esp8266at->io_mutex);

	return err;
}

esp8266at_err_t esp8266at_io_write(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written) {
	return esp8266at_io_write_advan(esp8266at, buffer, length, written, 0, 0, NULL);
}

esp8266at_err_t esp8266at_io_write_timedms(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	return esp8266at_io_write_advan(esp8266at, buffer, length, written, ESP8266AT_IO_OPTION__TIMED, timeoutms, remain_timeoutms);
}

esp8266at_err_t esp8266at_io_write_advan(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written, uint16_t io_option, uint32_t timeoutms, uint32_t* remain_timeoutms) {
	int r;
	esp8266at_err_t err;
	HAL_StatusTypeDef status;
	tickcount_t start_tick, end_tick, diff_tick;

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		r = mutex_lock_timedms(esp8266at->io_mutex, timeoutms);
		if (r == UBIK_ERR__TIMEOUT) {
			return ESP8266AT_TIMEOUT;
		}
		timeoutms = task_getremainingtimeoutms();
	} else {
		r = mutex_lock(esp8266at->io_mutex);
		timeoutms = 1000;
	}
	if (r != 0) {
		return ESP8266AT_ERROR;
	}

	for (;;) {
		start_tick = ubik_gettickcount();
		status = HAL_UART_Transmit(&_esp8266at_uart, buffer, length, timeoutms);
		end_tick = ubik_gettickcount();
		diff_tick = ubik_gettickdiff(start_tick, end_tick);
		timeoutms -= min(timeoutms, ubik_ticktotimems(diff_tick.low));

		if (status == HAL_TIMEOUT) {
			if ((io_option & ESP8266AT_IO_OPTION__TIMED) == 0) {
				continue;
			}
			if (written != NULL) {
				*written = 0;
			}
			err = ESP8266AT_TIMEOUT;
			break;
		} else if (status == HAL_OK) {
			if (written != NULL) {
				*written = length;
			}
			err = ESP8266AT_OK;
			break;
		} else {
			err = ESP8266AT_IO_ERROR;
			break;
		}
	}

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		if (remain_timeoutms) {
			*remain_timeoutms = timeoutms;
		}
	}

	mutex_unlock(esp8266at->io_mutex);

	return err;
}

void esp8266at_io_callback(void) {
	uint16_t tail;
	int need_signal = 0;

	esp8266at_t *esp8266at = &_esp8266at;

	tail = (esp8266at->io_read_buffer.tail + 1) % ESP8266AT_IO_READ_BUFFER_SIZE;

	if (HAL_UART_Receive_IT(&_esp8266at_uart, &esp8266at->io_read_buffer.data[tail], 1) == HAL_OK) {
		if (esp8266at->io_read_buffer.tail == esp8266at->io_read_buffer.head) {
			need_signal = 1;
		}
		esp8266at->io_read_buffer.tail = tail;
		if (need_signal) {
			sem_give(esp8266at->io_read_sem);
		}
	}
}

