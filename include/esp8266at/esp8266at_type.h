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

#include <ubinos.h>

#include <stdint.h>

#define ESP8266AT_VERSION_LENGTH_MAX 15
#define ESP8266AT_IP_ADDR_LENGTH_MAX 31
#define ESP8266AT_MAC_ADDR_LENGTH_MAX 31

#define ESP8266AT_SSID_LENGTH_MAX 64
#define ESP8266AT_PASSWD_LENGTH_MAX 64

#define ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX 64
#define ESP8266AT_DNS_SERVER_MAX 3

#define ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX 64
#define ESP8266AT_SNTP_SERVER_MAX 3

#define ESP8266AT_MQTT_CLIENT_ID_LENGTH_MAX 64
#define ESP8266AT_MQTT_USERNAME_LENGTH_MAX 64
#define ESP8266AT_MQTT_PASSWD_LENGTH_MAX 64

#define ESP8266AT_TEMP_CMD_BUF_SIZE 256
#define ESP8266AT_TEMP_RESP_BUF_SIZE 256

#define ESP8266AT_RESTART_SETUP_TIME_MS 2000

#define ESP8266AT_IO_OPTION__TIMED 0x0001

#define ESP8266AT_IO_DATA_KEY "+IPD,"
#define ESP8266AT_IO_DATA_KEY_LEN 5
#define ESP8266AT_IO_DATA_LEN_MAX 65536

#define ESP8266AT_IO_TEMP_RX_BUF_SIZE 1
#define ESP8266AT_IO_DATA_LEN_BUF_SIZE 256

#define ESP8266AT_IO_READ_BUF_SIZE 2048
#define ESP8266AT_IO_WRITE_BUF_SIZE 2048

#define ESP8266AT_IO_DATA_BUF_SIZE 256

#define ESP8266AT_IO_MQTT_KEY "+MQTTSUBRECV:0,"
#define ESP8266AT_IO_MQTT_KEY_LEN 15

#define ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX 128
#define ESP8266AT_IO_MQTT_SUB_DATA_BUF_SIZE 1024
#define ESP8266AT_IO_MQTT_SUB_BUF_MAX 3 // It must be greater or equal 3
#define ESP8266AT_IO_MQTT_SUB_BUF_MSG_MAX 5

typedef enum
{
    ESP8266AT_IO_RX_MODE_RESP = 0,
    ESP8266AT_IO_RX_MODE_DATA_LEN,
    ESP8266AT_IO_RX_MODE_DATA,
    ESP8266AT_IO_RX_MODE_MQTT_TOPIC,
} esp8266at_io_rx_mode_t;

typedef uint32_t esp8266at_mqtt_sub_buf_msg_t;
typedef struct _esp8266at_mqtt_sub_buf_t
{
    char topic[ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX];
    msgq_pt msgs;
    cbuf_pt data_buf;
    mutex_pt data_mutex;
} esp8266at_mqtt_sub_buf_t;

typedef struct _esp8266at_t
{
    char version[ESP8266AT_VERSION_LENGTH_MAX + 1];
    char ip_addr[ESP8266AT_IP_ADDR_LENGTH_MAX + 1];
    char mac_addr[ESP8266AT_MAC_ADDR_LENGTH_MAX + 1];

    mutex_pt cmd_mutex;

    char temp_cmd_buf[ESP8266AT_TEMP_CMD_BUF_SIZE];
    uint8_t temp_resp_buf[ESP8266AT_TEMP_RESP_BUF_SIZE];

    uint32_t rx_overflow_count;
    uint8_t tx_busy;

    mutex_pt io_mutex;
    sem_pt io_read_sem;
    cbuf_pt io_read_buf;
    sem_pt io_write_sem;
    cbuf_pt io_write_buf;

    uint8_t io_temp_rx_buf[ESP8266AT_IO_TEMP_RX_BUF_SIZE];
    uint8_t io_data_len_buf[ESP8266AT_IO_DATA_LEN_BUF_SIZE];

    int io_rx_mode;
    uint32_t io_data_key_i;
    uint32_t io_data_len;
    uint32_t io_data_len_i;
    uint32_t io_data_read;

    mutex_pt io_data_read_mutex;
    sem_pt io_data_read_sem;
    cbuf_pt io_data_buf;

    uint8_t cancel_interactive_mode;

    char ssid[ESP8266AT_SSID_LENGTH_MAX];
    char passwd[ESP8266AT_PASSWD_LENGTH_MAX];

    uint8_t mux_mode;

    uint8_t dns_enable;
    char dns_server_addr[ESP8266AT_DNS_SERVER_MAX][ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX];

    uint8_t sntp_enable;
    int8_t sntp_timezone;
    char sntp_server_addr[ESP8266AT_SNTP_SERVER_MAX][ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX];

    uint8_t mqtt_scheme;
    char mqtt_client_id[ESP8266AT_MQTT_CLIENT_ID_LENGTH_MAX];
    char mqtt_username[ESP8266AT_MQTT_USERNAME_LENGTH_MAX];
    char mqtt_passwd[ESP8266AT_MQTT_PASSWD_LENGTH_MAX];

    esp8266at_mqtt_sub_buf_t mqtt_sub_bufs[ESP8266AT_IO_MQTT_SUB_BUF_MAX];

    uint8_t io_mqtt_topic_buf[ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX];

    uint8_t io_is_mqtt;
    int32_t io_mqtt_sub_buf_id;
    uint32_t io_mqtt_key_i;
    uint32_t io_mqtt_topic_i;
} esp8266at_t;

/* Deprecated */
#define esp8266at_err_t ubi_st_t
#define ESP8266AT_ERR_OK                UBI_ST_OK
#define ESP8266AT_ERR_ERROR             UBI_ST_ERR
#define ESP8266AT_ERR_TIMEOUT           UBI_ST_TIMEOUT
#define ESP8266AT_ERR_BUSY              UBI_ST_BUSY
#define ESP8266AT_ERR_IO_ERROR          UBI_ST_ERR_IO
#define ESP8266AT_ERR_IO_OVERFLOW       UBI_ST_ERR_OVERFLOW

#ifdef __cplusplus
}
#endif

#endif /* ESP8266AT_TYPE_H_ */

