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

#if (INCLUDE__ESP8266AT == 1)

#include "esp8266at_io.h"

#undef LOGM_CATEGORY
#define LOGM_CATEGORY LOGM_CATEGORY__USER00

static esp8266at_err_t _wait_rsp(esp8266at_t *esp8266at, char *rsp, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms,
        uint32_t *remain_timeoutms);
static esp8266at_err_t _send_cmd_and_wait_rsp(esp8266at_t *esp8266at, char *cmd, char *rsp, uint32_t timeoutms, uint32_t *remain_timeoutms);

esp8266at_err_t esp8266at_init(esp8266at_t *esp8266at)
{
    int r;
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;

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

    return esp_err;
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

esp8266at_err_t esp8266at_cmd_at_rst(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t err;


#if (UBINOS__BSP__NRF52_NRF52XXX == 1)
    (void) r;
    err = ESP8266AT_ERR_OK;
#else
    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    esp8266at->io_rx_mode = ESP8266AT_IO_RX_MODE_RESP;
    esp8266at->io_data_key_i = 0;

    err = _send_cmd_and_wait_rsp(esp8266at, "AT+RST\r\n", "OK\r\n", timeoutms, &timeoutms);

    task_sleepms(ESP8266AT_RESTART_SETUP_TIME_MS);
    if (timeoutms < ESP8266AT_RESTART_SETUP_TIME_MS)
    {
        timeoutms = 0;
    }
    else
    {
        timeoutms -= ESP8266AT_RESTART_SETUP_TIME_MS;
    }

    if (remain_timeoutms)
    {
        *remain_timeoutms = timeoutms;
    }

    mutex_unlock(esp8266at->cmd_mutex);
#endif /* (UBINOS__BSP__NRF52_NRF52XXX == 1) */

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

    r = mutex_lock_timedms(esp8266at->cmd_mutex, timeoutms);
    timeoutms = task_getremainingtimeoutms();
    if (r == UBIK_ERR__TIMEOUT)
    {
        return ESP8266AT_ERR_TIMEOUT;
    }

    sprintf(esp8266at->temp_cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
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
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key));

            ptr2 = strstr(ptr1, "\"");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
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
                break;
            }
            ptr1 = (char*) (((unsigned int) ptr1) + strlen(key2));

            ptr2 = strstr(ptr1, "\"");
            if (ptr2 == NULL || ptr1 >= ptr2)
            {
                break;
            }

            size = (unsigned int) ptr2 - (unsigned int) ptr1;
            size = min(size, ESP8266AT_MAC_ADDR_LENGTH_MAX);
            strncpy(esp8266at->mac_addr, ptr1, size);

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

esp8266at_err_t esp8266at_cmd_at_ciprecv(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *received, uint32_t timeoutms,
        uint32_t *remain_timeoutms)
{
    int r;
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    uint32_t read_tmp;
    uint32_t read_tmp2;

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
            assert(r == 0);
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

#endif /* (INCLUDE__ESP8266AT == 1) */

