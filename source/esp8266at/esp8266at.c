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
#include <time.h>

#if (INCLUDE__ESP8266AT == 1)

#include "esp8266at_io.h"

#undef LOGM_CATEGORY
#define LOGM_CATEGORY ESP8266AT__LOGM_CATEGORY

#if (ESP8266AT__USE_WIZFI360_API == 1)
    #define _CIPDNS_STRING "CIPDNS_CUR"
    #define _CWJAP_STRING "CWJAP_CUR"
#else
    #define _CIPDNS_STRING "CIPDNS"
    #define _CWJAP_STRING "CWJAP"
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

static esp8266at_err_t _wait_rsp(esp8266at_t *esp8266at, char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms,
        uint32_t *remain_timeoutms);
static esp8266at_err_t _send_cmd_and_wait_rsp(esp8266at_t *esp8266at, char *cmd, char *rsp, uint32_t timeoutms, uint32_t *remain_timeoutms);

static void _esp8266at_interactive_recvfunc(void *arg);

esp8266at_err_t esp8266at_init(esp8266at_t *esp8266at)
{
    int r;
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    (void) r;
    (void) ubi_err;

    assert(esp8266at != NULL);
    assert(esp8266at->cmd_mutex == NULL);

    r = mutex_create(&esp8266at->cmd_mutex);
    assert(r == 0);

    esp8266at->rx_overflow_count = 0;
    esp8266at->tx_busy = 0;

    r = mutex_create(&esp8266at->io_mutex);
    assert(r == 0);

    r = semb_create(&esp8266at->io_read_sem);
    assert(r == 0);
    ubi_err = cbuf_create(&esp8266at->io_read_buf, ESP8266AT_IO_READ_BUF_SIZE);
    assert(ubi_err == UBI_ERR_OK);

    r = semb_create(&esp8266at->io_write_sem);
    assert(r == 0);
    ubi_err = cbuf_create(&esp8266at->io_write_buf, ESP8266AT_IO_WRITE_BUF_SIZE);
    assert(ubi_err == UBI_ERR_OK);

    esp8266at->io_rx_mode = ESP8266AT_IO_RX_MODE_RESP;
    esp8266at->io_data_key_i = 0;
    esp8266at->io_data_len = 0;
    esp8266at->io_data_len_i = 0;
    esp8266at->io_data_read = 0;

    r = mutex_create(&esp8266at->io_data_read_mutex);
    assert(r == 0);
    r = semb_create(&esp8266at->io_data_read_sem);
    assert(r == 0);
    ubi_err = cbuf_create(&esp8266at->io_data_buf, ESP8266AT_IO_DATA_BUF_SIZE);
    assert(ubi_err == UBI_ERR_OK);

    esp_err = esp8266at_io_init(esp8266at);
    assert(esp_err == ESP8266AT_ERR_OK);

    memset(esp8266at->ssid, 0, ESP8266AT_SSID_LENGTH_MAX);
    memset(esp8266at->passwd, 0, ESP8266AT_PASSWD_LENGTH_MAX);

    esp8266at->mux_mode = 0;

    esp8266at->dns_enable = 0;
    memset(esp8266at->dns_server_addr, 0, ESP8266AT_DNS_SERVER_MAX * ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX);

    esp8266at->sntp_enable = 0;
    esp8266at->sntp_timezone = 0;
    memset(esp8266at->sntp_server_addr, 0, ESP8266AT_SNTP_SERVER_MAX * ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX);

    esp8266at->mqtt_scheme = 1; // MQTT over TCP
    memset(esp8266at->mqtt_client_id, 0, ESP8266AT_MQTT_CLIENT_ID_LENGTH_MAX);
    memset(esp8266at->mqtt_username, 0, ESP8266AT_MQTT_USERNAME_LENGTH_MAX);
    memset(esp8266at->mqtt_passwd, 0, ESP8266AT_MQTT_PASSWD_LENGTH_MAX);

    for (int i = 0; i < ESP8266AT_IO_MQTT_SUB_BUF_MAX; i++)
    {
        memset(esp8266at->mqtt_sub_bufs[i].topic, 0, ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX);
        ubi_err = msgq_create(&esp8266at->mqtt_sub_bufs[i].msgs, sizeof(esp8266at_mqtt_sub_buf_msg_t), ESP8266AT_IO_MQTT_SUB_BUF_MSG_MAX);
        assert(ubi_err == UBI_ERR_OK);
        ubi_err = cbuf_create(&esp8266at->mqtt_sub_bufs[i].data_buf, ESP8266AT_IO_MQTT_SUB_DATA_BUF_SIZE);
        assert(ubi_err == UBI_ERR_OK);
        ubi_err = mutex_create(&esp8266at->mqtt_sub_bufs[i].data_mutex);
        assert(ubi_err == UBI_ERR_OK);
    }

    esp8266at->io_mqtt_key_i = 0;
    esp8266at->io_mqtt_topic_i = 0;

    esp_err = ESP8266AT_ERR_OK;

    return esp_err;
}

esp8266at_err_t esp8266at_deinit(esp8266at_t *esp8266at)
{
    esp8266at_err_t esp_err;

    assert(esp8266at != NULL);
    assert(esp8266at->cmd_mutex != NULL);

    esp_err = esp8266at_io_deinit(esp8266at);

    mutex_delete(&esp8266at->io_data_read_mutex);
    sem_delete(&esp8266at->io_data_read_sem);
    cbuf_delete(&esp8266at->io_data_buf);

    sem_delete(&esp8266at->io_write_sem);
    cbuf_delete(&esp8266at->io_write_buf);

    sem_delete(&esp8266at->io_read_sem);
    cbuf_delete(&esp8266at->io_read_buf);

    mutex_delete(&esp8266at->io_mutex);

    mutex_delete(&esp8266at->cmd_mutex);

    for (int i = 0; i < ESP8266AT_IO_MQTT_SUB_BUF_MAX; i++)
    {
        cbuf_delete(&esp8266at->mqtt_sub_bufs[i].data_buf);
        msgq_delete(&esp8266at->mqtt_sub_bufs[i].msgs);
        mutex_delete(&esp8266at->mqtt_sub_bufs[i].data_mutex);
    }

    return esp_err;
}

esp8266at_err_t esp8266at_reset(esp8266at_t *esp8266at)
{
    esp8266at_err_t err;

    esp8266at_io_module_reset(esp8266at);
    esp8266at_io_uart_reset(esp8266at);

    err = ESP8266AT_ERR_OK;

    return err;
}

static esp8266at_err_t _wait_rsp(esp8266at_t *esp8266at, char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms,
        uint32_t *remain_timeoutms)
{
    esp8266at_err_t err;
    uint32_t read;
    uint32_t rsp_len;
    uint32_t rsp_i;
    uint32_t buf_i;

    err = ESP8266AT_ERR_ERROR;

    logmd("wait response : begin");
    logmfd("wait response : \"%s\"", rsp);

    do
    {
        if (received)
        {
            *received = 0;
        }

        rsp_len = strlen(rsp);
        rsp_i = 0;
        buf_i = 0;
        while ((rsp_i < rsp_len) && (buf_i < length - 1))
        {
            err = esp8266at_io_read_timedms(esp8266at, &buffer[buf_i], 1, &read, timeoutms, &timeoutms);
            if (err != ESP8266AT_ERR_OK)
            {
                break;
            }
            if (read != 1)
            {
                err = ESP8266AT_ERR_ERROR;
                break;
            }

            if (rsp[rsp_i] == buffer[buf_i])
            {
                rsp_i++;
            }
            else
            {
                rsp_i = 0;
            }

            buf_i++;
        }
        buffer[buf_i] = 0;

        if (rsp_i != rsp_len)
        {
            err = ESP8266AT_ERR_ERROR;
            break;
        }

        if (received)
        {
            *received = buf_i;
        }

        err = ESP8266AT_ERR_OK;

        break;
    } while (1);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    logmfd("wait response : err = %d, size = %d, data = \"%s\"", err, buf_i, buffer);
    logmd("wait response : end");

    return err;
}

static esp8266at_err_t _send_cmd_and_wait_rsp(esp8266at_t *esp8266at, char *cmd, char *rsp, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    esp8266at_err_t err;

    err = ESP8266AT_ERR_ERROR;

    logmd("send command : begin");
    logmfd("send command : command = \"%s\", expected response = \"%s\"", cmd, rsp);

    do
    {
        err = esp8266at_io_read_buf_clear_timedms(esp8266at, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        err = esp8266at_io_write_timedms(esp8266at, (uint8_t*) cmd, strlen(cmd), NULL, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }
        err = esp8266at_io_flush_timedms(esp8266at, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        err = _wait_rsp(esp8266at, rsp, esp8266at->temp_resp_buf, ESP8266AT_TEMP_RESP_BUF_SIZE, NULL, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        break;
    } while (1);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    logmfd("send command : err = %d", err);
    logmd("send command : end");

    return err;
}

static void _esp8266at_interactive_recvfunc(void *arg)
{
    esp8266at_t *esp8266at = (esp8266at_t *) arg;
    uint8_t data;
    uint32_t read;

    while (esp8266at->cancel_interactive_mode == 0)
    {
        esp8266at_io_read_timedms(esp8266at, &data, 1, &read, 1000, NULL);
        if (read == 1) {
            dtty_putc(data);
            dtty_flush();
        }
    }
}

esp8266at_err_t esp8266at_cmd_at_interactive(esp8266at_t *esp8266at)
{
    int r;
    esp8266at_err_t err;
    task_pt recv_task;
    char data[2];
    int len;
    int old_echo;
    uint8_t ignore_lf = 0;

    err = ESP8266AT_ERR_ERROR;

    r = mutex_lock(esp8266at->cmd_mutex);
    if (r != 0)
    {
        return ESP8266AT_ERR_ERROR;
    }

    do {
        esp8266at->cancel_interactive_mode = 0;
        r = task_create_noautodel(&recv_task, _esp8266at_interactive_recvfunc, esp8266at, task_getmiddlepriority(), 0, "esp8266at_i_recv");
        if (r != 0)
        {
            err = ESP8266AT_ERR_ERROR;
            break;
        }
        old_echo = dtty_getecho();
        dtty_setecho(0);

        while (1)
        {
            r = dtty_getc(data);
            len = 1;
            if (r != 0)
            {
                continue;
            }
            if (data[0] == '\033') // ESC
            {
                err = ESP8266AT_ERR_OK;
                break;
            }
            if (ignore_lf)
            {
                ignore_lf = 0;
                if (data[0] == '\n')
                {
                    continue;
                }
            }
            if (data[0] == '\r' || data[0] == '\n')
            {
                if (data[0] == '\r')
                {
                    ignore_lf = 1;
                }
                data[0] = '\r';
                data[1] = '\n';
                len = 2;
            }

            err = esp8266at_io_write_timedms(esp8266at, (uint8_t*) data, len, NULL, UINT32_MAX, NULL);
            if (err != ESP8266AT_ERR_OK)
            {
                break;
            }
            err = esp8266at_io_flush_timedms(esp8266at, UINT32_MAX, NULL);
            if (err != ESP8266AT_ERR_OK)
            {
                break;
            }
        }

        dtty_setecho(old_echo);
        esp8266at->cancel_interactive_mode = 1;
        r = task_join_and_delete(&recv_task, NULL, 1);
        if (r != 0)
        {
            err = ESP8266AT_ERR_ERROR;
            break;
        }

        break;
    } while (1);

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_test(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_gmr(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    int size = 0;
    char *key = "AT version:";

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+GMR\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (err == ESP8266AT_ERR_OK)
    {
        do
        {
            ptr1 = strstr((char*) esp8266at->temp_resp_buf, key);
            if (ptr1 == NULL)
            {
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key));

            ptr2 = strstr(ptr1, "(");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }

            size = (unsigned int) ptr2 - (unsigned int) ptr1;
            size = min(size, ESP8266AT_VERSION_LENGTH_MAX);
            strncpy(esp8266at->version, ptr1, size);

            break;
        } while (1);
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_e(esp8266at_t *esp8266at, int is_on, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "ATE%d\r\n", is_on);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    task_sleepms(100);
    if (timeoutms < 100)
    {
        timeoutms = 0;
    }
    else
    {
        timeoutms -= 100;
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cwmode(esp8266at_t *esp8266at, int mode, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+CWMODE=%d\r\n", mode);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    task_sleepms(100);
    if (timeoutms < 100)
    {
        timeoutms = 0;
    }
    else
    {
        timeoutms -= 100;
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipmux(esp8266at_t *esp8266at, int mode, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+CIPMUX=%d\r\n", mode);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);
    if (err == ESP8266AT_ERR_OK)
    {
        esp8266at->mux_mode = mode;
    }

    task_sleepms(100);
    if (timeoutms < 100)
    {
        timeoutms = 0;
    }
    else
    {
        timeoutms -= 100;
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cwjap(esp8266at_t *esp8266at, char *ssid, char *passwd, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    assert(ssid != NULL);
    assert(passwd != NULL);

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    if (esp8266at->ssid != ssid)
    {
        strncpy(esp8266at->ssid, ssid, ESP8266AT_SSID_LENGTH_MAX);
    }
    if (esp8266at->passwd != passwd)
    {
        strncpy(esp8266at->passwd, passwd, ESP8266AT_PASSWD_LENGTH_MAX);
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+"_CWJAP_STRING"=\"%s\",\"%s\"\r\n", ssid, passwd);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cwqap(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    task_sleepms(500);
    if (timeoutms < 500)
    {
        timeoutms = 0;
    }
    else
    {
        timeoutms -= 500;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+CWQAP\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cifsr(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    int size = 0;
    char *key = "STAIP,\"";
    char *key2 = "STAMAC,\"";

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+CIFSR\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (err == ESP8266AT_ERR_OK)
    {
        do
        {
            ptr1 = strstr((char*) esp8266at->temp_resp_buf, key);
            if (ptr1 == NULL)
            {
                err = ESP8266AT_ERR_ERROR;
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key));

            ptr2 = strstr(ptr1, "\"");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                err = ESP8266AT_ERR_ERROR;
                break;
            }

            size = (unsigned int) ptr2 - (unsigned int) ptr1;
            size = min(size, ESP8266AT_IP_ADDR_LENGTH_MAX);
            strncpy(esp8266at->ip_addr, ptr1, size);

            break;
        } while (1);

        do
        {
            ptr1 = strstr((char*) esp8266at->temp_resp_buf, key2);
            if (ptr1 == NULL)
            {
                err = ESP8266AT_ERR_ERROR;
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key2));

            ptr2 = strstr(ptr1, "\"");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                err = ESP8266AT_ERR_ERROR;
                break;
            }

            size = (unsigned int) ptr2 - (unsigned int) ptr1;
            size = min(size, ESP8266AT_MAC_ADDR_LENGTH_MAX);
            strncpy(esp8266at->mac_addr, ptr1, size);

            break;
        } while (1);
    }

    if (err != ESP8266AT_ERR_OK)
    {
        memset(esp8266at->ip_addr, 0, ESP8266AT_IP_ADDR_LENGTH_MAX);
        memset(esp8266at->mac_addr, 0, ESP8266AT_MAC_ADDR_LENGTH_MAX);
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipstart(esp8266at_t *esp8266at, char *type, char *ip, uint32_t port, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+CIPSTART=\"%s\",\"%s\",%lu\r\n", type, ip, port);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipstart_multiple(esp8266at_t *esp8266at, int id, char *type, char *ip, uint32_t port, uint32_t timeoutms,
        uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+CIPSTART=%d,\"%s\",\"%s\",%lu\r\n", id, type, ip, port);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipclose(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+CIPCLOSE\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipsend(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = ESP8266AT_ERR_ERROR;

    do
    {
        sprintf(esp8266at->temp_cmd_buf, "AT+CIPSEND=%lu\r\n", length);
        err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, ">", timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        err = esp8266at_io_write_timedms(esp8266at, buffer, length, NULL, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }
        err = esp8266at_io_flush_timedms(esp8266at, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        err = _wait_rsp(esp8266at, "SEND OK\r\n", esp8266at->temp_resp_buf, ESP8266AT_TEMP_RESP_BUF_SIZE, NULL, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        break;
    } while (1);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_ciprecv(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    uint32_t read_tmp;
    uint32_t read_tmp2;
    (void) ubi_err;

    r = mutex_lock_timedms(esp8266at->io_data_read_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    esp_err = ESP8266AT_ERR_ERROR;

    read_tmp = 0;
    read_tmp2 = 0;

    for (;;)
    {
        ubi_err = cbuf_read(esp8266at->io_data_buf, &buffer[read_tmp], length - read_tmp, &read_tmp2);
        assert(ubi_err == UBI_ERR_OK || ubi_err == UBI_ERR_BUF_EMPTY);
        read_tmp += read_tmp2;

        if (read_tmp >= length)
        {
            esp_err = ESP8266AT_ERR_OK;
            break;
        }
        else
        {
            if (timeoutms == 0)
            {
                esp_err = ESP8266AT_ERR_TIMEOUT;
                break;
            }
            r = sem_take_timedms(esp8266at->io_data_read_sem, timeoutms);
            timeoutms = task_getremainingtimeoutms();
            if (r == UBIK_ERR__TIMEOUT)
            {
                esp_err = ESP8266AT_ERR_TIMEOUT;
                break;
            }
            if (r != 0)
            {
                esp_err = ESP8266AT_ERR_ERROR;
                break;
            }
        }
    }

    if (received)
    {
        *received = read_tmp;
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->io_data_read_mutex);

    return esp_err;
}

esp8266at_err_t esp8266at_cmd_at_cipdns(esp8266at_t *esp8266at, uint8_t enable, char * dns_server_addr, char * dns_server_addr2, char * dns_server_addr3, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    esp8266at->dns_enable = enable;

    sprintf(esp8266at->temp_cmd_buf, "AT+"_CIPDNS_STRING"=%d", enable);
    ptr1 = esp8266at->temp_cmd_buf + strlen(esp8266at->temp_cmd_buf);
    if (dns_server_addr != NULL && strlen(dns_server_addr) > 0)
    {
        sprintf(ptr1, ",\"%s\"", dns_server_addr);
        ptr1 += strlen(ptr1);
        if (esp8266at->dns_server_addr[0] != dns_server_addr)
        {
            strncpy(esp8266at->dns_server_addr[0], dns_server_addr, ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX);
        }

        if (dns_server_addr2 != NULL && strlen(dns_server_addr2) > 0)
        {
            sprintf(ptr1, ",\"%s\"", dns_server_addr2);
            ptr1 += strlen(ptr1);
            if (esp8266at->dns_server_addr[1] != dns_server_addr2)
            {
                strncpy(esp8266at->dns_server_addr[1], dns_server_addr2, ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX);
            }

            if (dns_server_addr3 != NULL && strlen(dns_server_addr3) > 0)
            {
                sprintf(ptr1, ",\"%s\"", dns_server_addr3);
                ptr1 += strlen(ptr1);
                if (esp8266at->dns_server_addr[2] != dns_server_addr3)
                {
                    strncpy(esp8266at->dns_server_addr[2], dns_server_addr3, ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX);
                }
            }
        }
    }
    sprintf(ptr1, "\r\n");

    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipdns_q(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    int size = 0;
    const char *key = "+"_CIPDNS_STRING":";

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+"_CIPDNS_STRING"?\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (err == ESP8266AT_ERR_OK)
    {
        do
        {
            ptr1 = strstr((char*) esp8266at->temp_resp_buf, key);
            if (ptr1 == NULL)
            {
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key));

            ////
            ptr2 = strstr(ptr1, ",");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            esp8266at->dns_enable = atoi(ptr1);
            *ptr2 = ' ';

            ////
            for (int i = 0; i < ESP8266AT_DNS_SERVER_MAX; i++)
            {
                ptr1 = ptr2;
                ptr2 = strstr(ptr1, "\"");
                if (ptr2 == NULL || ptr1 >= ptr2)
                {
                    break;
                }
                *ptr2 = ' ';
                ptr1 = ptr2;
                ptr2 = strstr(ptr1, "\"");
                if (ptr2 == NULL || ptr1 >= ptr2)
                {
                    break;
                }
                *ptr2 = ' ';
                ptr1++;
                size = (unsigned int) ptr2 - (unsigned int) ptr1;
                size = min(size, ESP8266AT_DNS_SERVER_ADDR_LENGTH_MAX);
                strncpy(esp8266at->dns_server_addr[i], ptr1, size);
            }
            break;
        } while (1);
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipsntpcfg(esp8266at_t *esp8266at, uint8_t enable, int8_t timezone, char * sntp_server_addr, char * sntp_server_addr2, char * sntp_server_addr3, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    esp8266at->sntp_enable = enable;
    esp8266at->sntp_timezone = timezone;

    sprintf(esp8266at->temp_cmd_buf, "AT+CIPSNTPCFG=%d,%d", enable, timezone);
    ptr1 = esp8266at->temp_cmd_buf + strlen(esp8266at->temp_cmd_buf);
    if (sntp_server_addr != NULL && strlen(sntp_server_addr) > 0)
    {
        sprintf(ptr1, ",\"%s\"", sntp_server_addr);
        ptr1 += strlen(ptr1);
        if (esp8266at->sntp_server_addr[0] != sntp_server_addr)
        {
            strncpy(esp8266at->sntp_server_addr[0], sntp_server_addr, ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX);
        }

        if (sntp_server_addr2 != NULL && strlen(sntp_server_addr2) > 0)
        {
            sprintf(ptr1, ",\"%s\"", sntp_server_addr2);
            ptr1 += strlen(ptr1);
            if (esp8266at->sntp_server_addr[1] != sntp_server_addr2)
            {
                strncpy(esp8266at->sntp_server_addr[1], sntp_server_addr2, ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX);
            }

            if (sntp_server_addr3 != NULL && strlen(sntp_server_addr3) > 0)
            {
                sprintf(ptr1, ",\"%s\"", sntp_server_addr3);
                ptr1 += strlen(ptr1);
                if (esp8266at->sntp_server_addr[2] != sntp_server_addr3)
                {
                    strncpy(esp8266at->sntp_server_addr[2], sntp_server_addr3, ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX);
                }
            }
        }
    }
    sprintf(ptr1, "\r\n");

    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipsntpcfg_q(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    int size = 0;
    const char *key = "+CIPSNTPCFG:";

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+CIPSNTPCFG?\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (err == ESP8266AT_ERR_OK)
    {
        do
        {
            ptr1 = strstr((char*) esp8266at->temp_resp_buf, key);
            if (ptr1 == NULL)
            {
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key));

            ////
            ptr2 = strstr(ptr1, ",");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            esp8266at->sntp_enable = atoi(ptr1);
            *ptr2 = ' ';

            ////
            ptr1 = ptr2;
            ptr2 = strstr(ptr1, ",");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            ptr1++;
            esp8266at->sntp_timezone = atoi(ptr1);
            *ptr2 = ' ';

            ////
            for (int i = 0; i < ESP8266AT_SNTP_SERVER_MAX; i++)
            {
                ptr1 = ptr2;
                ptr2 = strstr(ptr1, "\"");
                if (ptr2 == NULL || ptr1 >= ptr2)
                {
                    break;
                }
                *ptr2 = ' ';
                ptr1 = ptr2;
                ptr2 = strstr(ptr1, "\"");
                if (ptr2 == NULL || ptr1 >= ptr2)
                {
                    break;
                }
                *ptr2 = ' ';
                ptr1++;
                size = (unsigned int) ptr2 - (unsigned int) ptr1;
                size = min(size, ESP8266AT_SNTP_SERVER_ADDR_LENGTH_MAX);
                strncpy(esp8266at->sntp_server_addr[i], ptr1, size);
            }
            break;
        } while (1);
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_cipsntptime(esp8266at_t *esp8266at, struct tm * tm_ptr, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    const char * key = "+CIPSNTPTIME:";
    const char * wday[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char * mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if (tm_ptr == NULL)
    {
        return ESP8266AT_ERR_ERROR;
    }

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+CIPSNTPTIME?\r\n", "OK\r\n", timeoutms, &timeoutms);

    if (err == ESP8266AT_ERR_OK)
    {
        err = ESP8266AT_ERR_ERROR;
        do
        {
            ptr1 = strstr((char*) esp8266at->temp_resp_buf, key);
            if (ptr1 == NULL)
            {
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key));

            ////
            ptr2 = strstr(ptr1, " ");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            tm_ptr->tm_wday = 0;
            for (int i = 0; i < 7; i++)
            {
                if(strcmp(ptr1, wday[i]) == 0)
                {
                    tm_ptr->tm_wday = i;
                    break;
                }
            }
            *ptr2 = ',';

            ////
            ptr1 = ptr2;
            ptr2 = strstr(ptr1, " ");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            ptr1++;
            *ptr2 = 0x0;
            tm_ptr->tm_mon = 0;
            for (int i = 0; i < 12; i++)
            {
                if(strcmp(ptr1, mon[i]) == 0)
                {
                    tm_ptr->tm_mon = i;
                    break;
                }
            }
            *ptr2 = ',';

            ////
            ptr1 = ptr2;
            ptr2 = strstr(ptr1, " ");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            ptr1++;
            tm_ptr->tm_mday = atoi(ptr1);
            *ptr2 = ',';

            ////
            ptr1 = ptr2;
            ptr2 = strstr(ptr1, ":");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            ptr1++;
            tm_ptr->tm_hour = atoi(ptr1);
            *ptr2 = ',';

            ////
            ptr1 = ptr2;
            ptr2 = strstr(ptr1, ":");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            ptr1++;
            tm_ptr->tm_min = atoi(ptr1);
            *ptr2 = ',';

            ////
            ptr1 = ptr2;
            ptr2 = strstr(ptr1, " ");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }
            *ptr2 = 0x0;
            ptr1++;
            tm_ptr->tm_sec = atoi(ptr1);
            *ptr2 = ' ';

            ////
            ptr1 = ptr2;
            tm_ptr->tm_year = atoi(ptr1);
            if (tm_ptr->tm_year < 1900)
            {
                break;
            }
            tm_ptr->tm_year -= 1900;

            ////
            err = ESP8266AT_ERR_OK;
            break;
        } while (1);
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttusercfg(esp8266at_t *esp8266at, uint8_t mqtt_scheme, char * mqtt_client_id, char * mqtt_username, char * mqtt_passwd, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    if (mqtt_client_id == NULL || mqtt_username == NULL || mqtt_passwd == NULL)
    {
        return ESP8266AT_ERR_ERROR;
    }

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    esp8266at->mqtt_scheme = mqtt_scheme;
    if (esp8266at->mqtt_client_id != mqtt_client_id)
    {
        strncpy(esp8266at->mqtt_client_id, mqtt_client_id, ESP8266AT_MQTT_CLIENT_ID_LENGTH_MAX);
    }
    if (esp8266at->mqtt_username != mqtt_username)
    {
        strncpy(esp8266at->mqtt_username, mqtt_username, ESP8266AT_MQTT_USERNAME_LENGTH_MAX);
    }
    if (esp8266at->mqtt_passwd != mqtt_passwd)
    {
        strncpy(esp8266at->mqtt_passwd, mqtt_passwd, ESP8266AT_MQTT_PASSWD_LENGTH_MAX);
    }

#if (ESP8266AT__USE_WIZFI360_API == 1)
    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTSET=\"%s\",\"%s\",\"%s\",60\r\n", 
        mqtt_username, mqtt_passwd, mqtt_client_id);
#else
    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTUSERCFG=0,%d,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", 
        mqtt_scheme, mqtt_client_id, mqtt_username, mqtt_passwd);
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

#if (ESP8266AT__USE_WIZFI360_API == 1)
esp8266at_err_t esp8266at_cmd_at_mqtttopic(esp8266at_t *esp8266at, char *pub_topic, char *sub_topic, char *sub_topic_2, char *sub_topic_3, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    char *ptr1;

    if (pub_topic == NULL || pub_topic == NULL || sub_topic == NULL)
    {
        return ESP8266AT_ERR_ERROR;
    }

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTTOPIC=");
    ptr1 = esp8266at->temp_cmd_buf + strlen(esp8266at->temp_cmd_buf);

    sprintf(ptr1, "\"%s\"", pub_topic);
    ptr1 += strlen(ptr1);

    sprintf(ptr1, ",\"%s\"", sub_topic);
    ptr1 += strlen(ptr1);
    strncpy(esp8266at->mqtt_sub_bufs[0].topic, sub_topic, ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX);    

    if (sub_topic_2 != NULL && strlen(sub_topic_2) > 0)
    {
        sprintf(ptr1, ",\"%s\"", sub_topic_2);
        ptr1 += strlen(ptr1);
        strncpy(esp8266at->mqtt_sub_bufs[1].topic, sub_topic, ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX);    
    }

    if (sub_topic_3 != NULL && strlen(sub_topic_3) > 0)
    {
        sprintf(ptr1, ",\"%s\"", sub_topic_3);
        ptr1 += strlen(ptr1);
        strncpy(esp8266at->mqtt_sub_bufs[2].topic, sub_topic, ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX);    
    }

    sprintf(ptr1, "\r\n");

    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}
#else
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

esp8266at_err_t esp8266at_cmd_at_mqttconn(esp8266at_t *esp8266at, char *ip, uint32_t port, uint32_t reconnect, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

#if (ESP8266AT__USE_WIZFI360_API == 1)
    if (esp8266at->mux_mode == 0)
    {
        sprintf(esp8266at->temp_cmd_buf, "AT+MQTTCON=0,\"%s\",%lu\r\n", ip, port);
        err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);
    }
    else
    {
        sprintf(esp8266at->temp_cmd_buf, "AT+MQTTCON=0,0,\"%s\",%lu\r\n", ip, port);
        err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);
    }
#else
    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTCONN=0,\"%s\",%lu,%lu\r\n", ip, port, reconnect);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttclean(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

#if (ESP8266AT__USE_WIZFI360_API == 1)
    err = _send_cmd_and_wait_rsp(esp8266at, "AT+MQTTDIS\r\n", "CLOSED\r\n", timeoutms, &timeoutms);
#else
    err = _send_cmd_and_wait_rsp(esp8266at, "AT+MQTTCLEAN=0\r\n", "OK\r\n", timeoutms, &timeoutms);
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttpub(esp8266at_t *esp8266at, char *topic, char *data, uint32_t qos, uint32_t retain, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

#if (ESP8266AT__USE_WIZFI360_API == 1)
    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTPUB=\"%s\"\r\n", data);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);
#else
    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTPUB=0,\"%s\",\"%s\",%lu,%lu\r\n", topic, data, qos, retain);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttpubraw(esp8266at_t *esp8266at, char *topic, char *data, uint32_t length, uint32_t qos, uint32_t retain, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;
    (void) r;

#if (ESP8266AT__USE_WIZFI360_API == 1)
    err = esp8266at_cmd_at_mqttpub(esp8266at, topic, data, qos, retain, timeoutms, remain_timeoutms);
#else
    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    err = ESP8266AT_ERR_ERROR;

    do
    {
        sprintf(esp8266at->temp_cmd_buf, "AT+MQTTPUBRAW=0,\"%s\",%lu,%lu,%lu\r\n", topic, length, qos, retain);
        err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, ">", timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        err = esp8266at_io_write_timedms(esp8266at, (uint8_t *) data, length, NULL, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }
        err = esp8266at_io_flush_timedms(esp8266at, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        err = _wait_rsp(esp8266at, "+MQTTPUB:OK\r\n", esp8266at->temp_resp_buf, ESP8266AT_TEMP_RESP_BUF_SIZE, NULL, timeoutms, &timeoutms);
        if (err != ESP8266AT_ERR_OK)
        {
            break;
        }

        break;
    } while (1);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);
#endif /* (ESP8266AT__USE_WIZFI360_API == 1) */

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttsub(esp8266at_t *esp8266at, uint32_t id, char *topic, uint32_t qos, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    if (id >= ESP8266AT_IO_MQTT_SUB_BUF_MAX)
    {
        return ESP8266AT_ERR_ERROR;
    }

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    strncpy(esp8266at->mqtt_sub_bufs[id].topic, topic, ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX);

    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTSUB=0,\"%s\",%lu\r\n", topic, qos);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttsub_q(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTSUB?\r\n");
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttunsub(esp8266at_t *esp8266at, uint32_t id, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;

    if (id >= ESP8266AT_IO_MQTT_SUB_BUF_MAX)
    {
        return ESP8266AT_ERR_ERROR;
    }

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+MQTTUNSUB=0,\"%s\"\r\n", esp8266at->mqtt_sub_bufs[id].topic);
    err = _send_cmd_and_wait_rsp(esp8266at, esp8266at->temp_cmd_buf, "OK\r\n", timeoutms, &timeoutms);

    if (err == ESP8266AT_ERR_OK)
    {
        memset(esp8266at->mqtt_sub_bufs[id].topic, 0, ESP8266AT_IO_MQTT_TOPIC_LENGTH_MAX);
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);

    return err;
}

esp8266at_err_t esp8266at_cmd_at_mqttsubget(esp8266at_t *esp8266at, uint32_t id, uint8_t *buffer, uint32_t max_length, uint32_t *received, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    uint32_t read_tmp = 0;
    uint32_t len;
    esp8266at_mqtt_sub_buf_t *sub_buf_p = NULL;
    esp8266at_mqtt_sub_buf_msg_t sub_msg;
    (void) ubi_err;

    if (id >= ESP8266AT_IO_MQTT_SUB_BUF_MAX)
    {
        return ESP8266AT_ERR_ERROR;
    }

    sub_buf_p = &esp8266at->mqtt_sub_bufs[id];

    r = mutex_lock_timedms(sub_buf_p->data_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    do
    {
        r = msgq_receive_timedms(sub_buf_p->msgs, (unsigned char *) &sub_msg, timeoutms);
        timeoutms = task_getremainingtimeoutms();
        if (r == UBIK_ERR__TIMEOUT)
        {
            esp_err = ESP8266AT_ERR_TIMEOUT;
            break;
        }
        if (r != 0)
        {
            esp_err = ESP8266AT_ERR_ERROR;
            break;
        }

        if (sub_msg > max_length)
        {
            len = max_length;
            ubi_err = cbuf_read(sub_buf_p->data_buf, buffer, len, &read_tmp);
            assert(ubi_err == UBI_ERR_OK);
            ubi_err = cbuf_read(sub_buf_p->data_buf, NULL, sub_msg - len, NULL);
            assert(ubi_err == UBI_ERR_OK);
            esp_err = ESP8266AT_ERR_IO_OVERFLOW;
        }
        else
        {
            len = sub_msg;
            ubi_err = cbuf_read(sub_buf_p->data_buf, buffer, len, &read_tmp);
            assert(ubi_err == UBI_ERR_OK);
            esp_err = ESP8266AT_ERR_OK;
        }

        break;
    } while (1);

    if (received)
    {
        *received = read_tmp;
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(sub_buf_p->data_mutex);

    return esp_err;
}

#endif /* (INCLUDE__ESP8266AT == 1) */

