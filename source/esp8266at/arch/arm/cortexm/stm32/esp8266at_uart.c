/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>
#include <assert.h>

#include "../../../../esp8266at_io.h"

#include "main.h"

void esp8266at_io_callback(void);

UART_HandleTypeDef _esp8266at_uart;

typedef struct {
	uint8_t data[ESP8266AT_IO_READ_BUFFER_SIZE];
	uint16_t head;
	uint16_t tail;
} _esp8266at_read_buffer_t;

static _esp8266at_read_buffer_t _read_buffer;

static sem_pt _read_sem = NULL;

static mutex_pt _io_mutex = NULL;

esp8266at_err_t esp8266at_io_init(void) {
	int r;
	esp8266at_err_t err;

	assert(_read_sem == NULL);
	assert(_io_mutex == NULL);

	err = ESP8266AT_ERROR;

	do {
		r = semb_create(&_read_sem);
		if (r != 0) {
			err = ESP8266AT_ERROR;
			break;
		}

		r = mutex_create(&_io_mutex);
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

		_read_buffer.head = 0;
		_read_buffer.tail = 0;

		HAL_UART_Receive_IT(&_esp8266at_uart, &_read_buffer.data[0], 1);

		err = ESP8266AT_OK;
	} while (0);

	if (err != ESP8266AT_OK) {
		if (_io_mutex != NULL) {
			mutex_delete(&_io_mutex);
		}
		if (_read_sem != NULL) {
			sem_delete(&_read_sem);
		}
	}

	return err;
}

esp8266at_err_t esp8266at_io_deinit(void) {
	esp8266at_err_t err = ESP8266AT_OK;

	assert(_read_sem != NULL);
	assert(_io_mutex != NULL);

	if (HAL_UART_DeInit(&_esp8266at_uart) != HAL_OK) {
		err = ESP8266AT_ERROR;
	}

	mutex_delete(&_io_mutex);
	sem_delete(&_read_sem);

	return err;
}

esp8266at_err_t esp8266at_io_read_clear(void) {
	return esp8266at_io_read_clear_advan(0, 0);
}

esp8266at_err_t esp8266at_io_read_clear_timedms(uint32_t timeoutms) {
	return esp8266at_io_read_clear_advan(ESP8266AT_IO_OPTION__TIMED, timeoutms);
}

esp8266at_err_t esp8266at_io_read_clear_advan(uint16_t io_option, uint32_t timeoutms) {
	int r;

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		r = mutex_lock_timedms(_io_mutex, timeoutms);
		if (r == UBIK_ERR__TIMEOUT) {
			return ESP8266AT_TIMEOUT;
		}
		timeoutms = task_getremainingtimeoutms();
	} else {
		r = mutex_lock(_io_mutex);
	}
	if (r != 0) {
		return ESP8266AT_ERROR;
	}

	_read_buffer.head = _read_buffer.tail;

	mutex_unlock(_io_mutex);

	return ESP8266AT_OK;
}

esp8266at_err_t esp8266at_io_read(uint8_t *buffer, uint32_t length, uint32_t *read) {
	return esp8266at_io_read_advan(buffer, length, read, 0, 0);
}

esp8266at_err_t esp8266at_io_read_timedms(uint8_t *buffer, uint32_t length, uint32_t *read, uint32_t timeoutms) {
	return esp8266at_io_read_advan(buffer, length, read, ESP8266AT_IO_OPTION__TIMED, timeoutms);
}

esp8266at_err_t esp8266at_io_read_advan(uint8_t *buffer, uint32_t length, uint32_t *read, uint16_t io_option, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;
	uint32_t read_tmp;
	int is_first;

	assert(buffer != NULL);

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		r = mutex_lock_timedms(_io_mutex, timeoutms);
		if (r == UBIK_ERR__TIMEOUT) {
			return ESP8266AT_TIMEOUT;
		}
		timeoutms = task_getremainingtimeoutms();
	} else {
		r = mutex_lock(_io_mutex);
	}
	if (r != 0) {
		return ESP8266AT_ERROR;
	}

	read_tmp = 0;
	is_first = 1;
	err = ESP8266AT_IO_ERROR;

	while (read_tmp < length) {
		if (_read_buffer.head != _read_buffer.tail) {
			buffer[read_tmp] = _read_buffer.data[_read_buffer.head];
			_read_buffer.head = (_read_buffer.head + 1) % ESP8266AT_IO_READ_BUFFER_SIZE;

			read_tmp++;
		} else {
			if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
				if (is_first == 0 && timeoutms == 0) {
					err = ESP8266AT_TIMEOUT;
					break;
				}
				r = sem_take_timedms(_read_sem, timeoutms);
				timeoutms = task_getremainingtimeoutms();
			} else {
				r = sem_take(_read_sem);
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

	mutex_unlock(_io_mutex);

	return err;
}

esp8266at_err_t esp8266at_io_write(uint8_t *buffer, uint32_t length, uint32_t *written) {
	return esp8266at_io_write_advan(buffer, length, written, 0, 0);
}

esp8266at_err_t esp8266at_io_write_timedms(uint8_t *buffer, uint32_t length, uint32_t *written, uint32_t timeoutms) {
	return esp8266at_io_write_advan(buffer, length, written, ESP8266AT_IO_OPTION__TIMED, timeoutms);
}

esp8266at_err_t esp8266at_io_write_advan(uint8_t *buffer, uint32_t length, uint32_t *written, uint16_t io_option, uint32_t timeoutms) {
	int r;
	esp8266at_err_t err;
	HAL_StatusTypeDef status;

	if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0) {
		r = mutex_lock_timedms(_io_mutex, timeoutms);
		if (r == UBIK_ERR__TIMEOUT) {
			return ESP8266AT_TIMEOUT;
		}
		timeoutms = task_getremainingtimeoutms();
	} else {
		r = mutex_lock(_io_mutex);
		timeoutms = 1000;
	}
	if (r != 0) {
		return ESP8266AT_ERROR;
	}

	while (1) {
		status = HAL_UART_Transmit(&_esp8266at_uart, buffer, length, timeoutms);

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

	mutex_unlock(_io_mutex);

	return err;
}

void esp8266at_io_callback(void) {
	uint16_t tail;

	tail = (_read_buffer.tail + 1) % ESP8266AT_IO_READ_BUFFER_SIZE;

	if (HAL_UART_Receive_IT(&_esp8266at_uart, &_read_buffer.data[tail], 1) == HAL_OK) {
		_read_buffer.tail = tail;
		sem_give(_read_sem);
	}
}

