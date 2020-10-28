/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP8266AT_TYPE_H_
#define ESP8266AT_TYPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * @file esp8266at_type.h
 *
 * @brief ESP8266 AT Type
 *
 * ESP8266 module과 AT 명령 통신을 하기 위한 형들을 정의합니다.
 */

#include <stdint.h>

#define ESP8266AT_IO_READ_BUFFER_SIZE 2048

#define ESP8266AT_CMD_BUFFER_SIZE 256
#define ESP8266AT_RSP_BUFFER_SIZE 256

#define ESP8266AT_VERSION_LENGTH_MAX 15
#define ESP8266AT_IP_ADDR_LENGTH_MAX 31
#define ESP8266AT_MAC_ADDR_LENGTH_MAX 31

typedef enum
{
    ESP8266AT_ERR_OK = 0,
    ESP8266AT_ERR_ERROR,
    ESP8266AT_ERR_TIMEOUT,
    ESP8266AT_ERR_BUSY,
    ESP8266AT_ERR_IO_ERROR,
} esp8266at_err_t;

typedef struct _esp8266at_read_buffer_t
{
    uint8_t data[ESP8266AT_IO_READ_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} esp8266at_read_buffer_t;

#define ESP8266AT_IO_OPTION__TIMED 0x0001

typedef struct _esp8266at_t
{
    char version[ESP8266AT_VERSION_LENGTH_MAX + 1];
    char ip_addr[ESP8266AT_IP_ADDR_LENGTH_MAX + 1];
    char mac_addr[ESP8266AT_MAC_ADDR_LENGTH_MAX + 1];

    char cmd_buf[ESP8266AT_CMD_BUFFER_SIZE];
    uint8_t rsp_buf[ESP8266AT_RSP_BUFFER_SIZE];
    mutex_pt cmd_mutex;

    esp8266at_read_buffer_t io_read_buffer;
    sem_pt io_read_sem;
    mutex_pt io_mutex;
} esp8266at_t;

#ifdef __cplusplus
}
#endif

#endif /* ESP8266AT_TYPE_H_ */

