/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>

#if (INCLUDE__ESP8266AT == 1)
#if (UBINOS__BSP__STM32_STM32XXXX == 1)

#if (INCLUDE__UBINOS__UBIK != 1)
	#error "ubik is necessary"
#endif

#include <assert.h>

#include "../../../../esp8266at_io.h"

#include "main.h"

#undef LOGM_CATEGORY
#define LOGM_CATEGORY LOGM_CATEGORY__USER00

void esp8266at_io_rx_callback(void);
void esp8266at_io_tx_callback(void);

extern esp8266at_t _g_esp8266at;

extern UART_HandleTypeDef ESP8266_UART_HANDLE;

void esp8266at_io_rx_callback(void)
{
    int need_signal = 0;
    uint8_t *buf;
    uint32_t len;
    cbuf_pt rbuf = _g_esp8266at.io_read_buf;
    sem_pt rsem = _g_esp8266at.io_read_sem;

    do
    {
        len = 1;

        if (cbuf_is_full(rbuf))
        {
            _g_esp8266at.rx_overflow_count++;
            break;
        }

        if (cbuf_get_len(rbuf) == 0)
        {
            need_signal = 1;
        }

        cbuf_write(rbuf, NULL, len, NULL);

        if (need_signal && _bsp_kernel_active)
        {
            sem_give(rsem);
        }

        buf = cbuf_get_tail_addr(rbuf);
        HAL_UART_Receive_IT(&ESP8266_UART_HANDLE, buf, len);
    } while (0);
}

void esp8266at_io_tx_callback(void)
{
    uint8_t *buf;
    uint32_t len;
    cbuf_pt wbuf = _g_esp8266at.io_write_buf;
    sem_pt wsem = _g_esp8266at.io_write_sem;

    do
    {
        len = 1;

        cbuf_read(wbuf, NULL, len, NULL);

        if (cbuf_get_len(wbuf) > 0)
        {
            buf = cbuf_get_head_addr(wbuf);
            len = 1;
            HAL_UART_Transmit_IT(&ESP8266_UART_HANDLE, buf, len);
        }
        else
        {
            _g_esp8266at.tx_busy = 0;
            sem_give(wsem);
        }
    } while (0);
}

esp8266at_err_t esp8266at_io_init(esp8266at_t *esp8266at) {
	esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    HAL_StatusTypeDef stm_err;
    int r;
    uint8_t *buf;
    uint32_t len;
    assert(esp8266at != NULL);
    (void) r;
    (void) ubi_err;
    (void) stm_err;

    esp8266at->rx_overflow_count = 0;
    esp8266at->tx_busy = 0;

    ubi_err = cbuf_create(&esp8266at->io_read_buf, ESP8266AT_IO_READ_BUF_SIZE);
    assert(ubi_err == UBI_ERR_OK);
    ubi_err = cbuf_create(&esp8266at->io_write_buf, ESP8266AT_IO_WRITE_BUF_SIZE);
    assert(ubi_err == UBI_ERR_OK);
    r = mutex_create(&esp8266at->io_mutex);
    assert(r == 0);
    r = semb_create(&esp8266at->io_read_sem);
    assert(r == 0);
    r = semb_create(&esp8266at->io_write_sem);
    assert(r == 0);

    ESP8266_UART_HANDLE.Instance = ESP8266_UART;
    ESP8266_UART_HANDLE.Init.BaudRate = 115200;
    ESP8266_UART_HANDLE.Init.WordLength = UART_WORDLENGTH_8B;
    ESP8266_UART_HANDLE.Init.StopBits = UART_STOPBITS_1;
    ESP8266_UART_HANDLE.Init.Parity = UART_PARITY_NONE;
    ESP8266_UART_HANDLE.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    ESP8266_UART_HANDLE.Init.Mode = UART_MODE_TX_RX;
    ESP8266_UART_HANDLE.Init.OverSampling = UART_OVERSAMPLING_16;
    stm_err = HAL_UART_Init(&ESP8266_UART_HANDLE);
    assert(stm_err == HAL_OK);

    if (!cbuf_is_full(esp8266at->io_read_buf))
    {
        buf = cbuf_get_tail_addr(esp8266at->io_read_buf);
        len = 1;
        HAL_UART_Receive_IT(&ESP8266_UART_HANDLE, buf, len);
    }

    esp_err = ESP8266AT_ERR_OK;

	return esp_err;
}

esp8266at_err_t esp8266at_io_deinit(esp8266at_t *esp8266at)
{
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    int r;
    assert(esp8266at != NULL);
    (void) r;
    (void) ubi_err;

    HAL_UART_DeInit(&ESP8266_UART_HANDLE);

    r = mutex_delete(&esp8266at->io_mutex);
    assert(r == 0);
    r = sem_delete(&esp8266at->io_read_sem);
    assert(r == 0);
    r = sem_delete(&esp8266at->io_write_sem);
    assert(r == 0);
    ubi_err = cbuf_delete(&esp8266at->io_read_buf);
    assert(ubi_err == UBI_ERR_OK);
    ubi_err = cbuf_delete(&esp8266at->io_write_buf);
    assert(ubi_err == UBI_ERR_OK);

    esp_err = ESP8266AT_ERR_OK;

    return esp_err;
}

esp8266at_err_t esp8266at_io_read_buf_clear(esp8266at_t *esp8266at)
{
    return esp8266at_io_read_buf_clear_advan(esp8266at, 0, 0, NULL);
}

esp8266at_err_t esp8266at_io_read_buf_clear_timedms(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    return esp8266at_io_read_buf_clear_advan(esp8266at, ESP8266AT_IO_OPTION__TIMED, timeoutms, remain_timeoutms);
}

esp8266at_err_t esp8266at_io_read_buf_clear_advan(esp8266at_t *esp8266at, uint16_t io_option, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    int r;
    assert(esp8266at != NULL);
    (void) r;
    (void) ubi_err;

    do
    {
        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            r = mutex_lock_timedms(esp8266at->io_mutex, timeoutms);
            timeoutms = task_getremainingtimeoutms();
            if (r == UBIK_ERR__TIMEOUT)
            {
                esp_err = ESP8266AT_ERR_TIMEOUT;
                break;
            }
            assert(r == 0);
        }
        else
        {
            r = mutex_lock(esp8266at->io_mutex);
            assert(r == 0);
        }

        ubi_err = cbuf_clear(esp8266at->io_read_buf);
        assert(ubi_err == UBI_ERR_OK);

        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            if (remain_timeoutms)
            {
                *remain_timeoutms = timeoutms;
            }
        }

        r = mutex_unlock(esp8266at->io_mutex);
        assert(r == 0);

        esp_err = ESP8266AT_ERR_OK;
    } while (0);

    return esp_err;
}

esp8266at_err_t esp8266at_io_flush(esp8266at_t *esp8266at)
{
    return esp8266at_io_flush_advan(esp8266at, 0, 0, NULL);
}

esp8266at_err_t esp8266at_io_flush_timedms(esp8266at_t *esp8266at, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    return esp8266at_io_flush_advan(esp8266at, ESP8266AT_IO_OPTION__TIMED, timeoutms, remain_timeoutms);
}

esp8266at_err_t esp8266at_io_flush_advan(esp8266at_t *esp8266at, uint16_t io_option, uint32_t timeoutms, uint32_t *remain_timeoutms)
{
    esp8266at_err_t esp_err;
    int r;
    assert(esp8266at != NULL);

    do
    {
        esp_err = ESP8266AT_ERR_OK;

        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            r = mutex_lock_timedms(esp8266at->io_mutex, timeoutms);
            timeoutms = task_getremainingtimeoutms();
            if (r == UBIK_ERR__TIMEOUT)
            {
                esp_err = ESP8266AT_ERR_TIMEOUT;
                break;
            }
            assert(r == 0);
        }
        else
        {
            r = mutex_lock(esp8266at->io_mutex);
            assert(r == 0);
        }

        for (;;)
        {
            if (cbuf_get_len(esp8266at->io_write_buf) == 0)
            {
                break;
            }
            r = sem_take_timedms(esp8266at->io_write_sem, timeoutms);
            timeoutms = task_getremainingtimeoutms();
            if (r == UBIK_ERR__TIMEOUT)
            {
                esp_err = ESP8266AT_ERR_TIMEOUT;
                break;
            }
        }

        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            if (remain_timeoutms)
            {
                *remain_timeoutms = timeoutms;
            }
        }

        r = mutex_unlock(esp8266at->io_mutex);
        assert(r == 0);
    } while (0);

    return esp_err;
}

esp8266at_err_t esp8266at_io_read(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read)
{
    return esp8266at_io_read_advan(esp8266at, buffer, length, read, 0, 0, NULL);
}

esp8266at_err_t esp8266at_io_read_timedms(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read, uint32_t timeoutms,
        uint32_t *remain_timeoutms)
{
    return esp8266at_io_read_advan(esp8266at, buffer, length, read, ESP8266AT_IO_OPTION__TIMED, timeoutms, remain_timeoutms);
}

esp8266at_err_t esp8266at_io_read_advan(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *read, uint16_t io_option, uint32_t timeoutms,
        uint32_t *remain_timeoutms)
{
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    int r;
    uint32_t read_tmp;
    uint32_t read_tmp2;
    assert(esp8266at != NULL);
    assert(buffer != NULL);
    (void) r;
    (void) ubi_err;

    do
    {
        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            r = mutex_lock_timedms(esp8266at->io_mutex, timeoutms);
            timeoutms = task_getremainingtimeoutms();
            if (r == UBIK_ERR__TIMEOUT)
            {
                esp_err = ESP8266AT_ERR_TIMEOUT;
                break;
            }
            assert(r == 0);
        }
        else
        {
            r = mutex_lock(esp8266at->io_mutex);
            assert(r == 0);
        }

        read_tmp = 0;
        read_tmp2 = 0;

        for (;;)
        {
            ubi_err = cbuf_read(esp8266at->io_read_buf, &buffer[read_tmp], length - read_tmp, &read_tmp2);
            assert(ubi_err == UBI_ERR_OK || ubi_err == UBI_ERR_BUF_EMPTY);
            read_tmp += read_tmp2;

            if (read_tmp >= length)
            {
                esp_err = ESP8266AT_ERR_OK;
                break;
            }
            else
            {
                if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
                {
                    if (timeoutms == 0)
                    {
                        esp_err = ESP8266AT_ERR_TIMEOUT;
                        break;
                    }
                    r = sem_take_timedms(esp8266at->io_read_sem, timeoutms);
                    timeoutms = task_getremainingtimeoutms();
                    if (r == UBIK_ERR__TIMEOUT)
                    {
                        esp_err = ESP8266AT_ERR_TIMEOUT;
                        break;
                    }
                    assert(r == 0);
                }
                else
                {
                    r = sem_take(esp8266at->io_read_sem);
                    assert(r == 0);
                }
            }
        }

        if (read)
        {
            *read = read_tmp;
        }

        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            if (remain_timeoutms)
            {
                *remain_timeoutms = timeoutms;
            }
        }

        r = mutex_unlock(esp8266at->io_mutex);
        assert(r == 0);
    } while (0);

    return esp_err;
}

esp8266at_err_t esp8266at_io_write(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written)
{
    return esp8266at_io_write_advan(esp8266at, buffer, length, written, 0, 0, NULL);
}

esp8266at_err_t esp8266at_io_write_timedms(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written, uint32_t timeoutms,
        uint32_t *remain_timeoutms)
{
    return esp8266at_io_write_advan(esp8266at, buffer, length, written, ESP8266AT_IO_OPTION__TIMED, timeoutms, remain_timeoutms);
}

esp8266at_err_t esp8266at_io_write_advan(esp8266at_t *esp8266at, uint8_t *buffer, uint32_t length, uint32_t *written, uint16_t io_option, uint32_t timeoutms,
        uint32_t *remain_timeoutms)
{
    esp8266at_err_t esp_err;
    ubi_err_t ubi_err;
    int r;
    uint8_t *buf;
    size_t len;
    uint32_t written_tmp;
    assert(esp8266at != NULL);
    assert(buffer != NULL);

    do
    {
        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            r = mutex_lock_timedms(esp8266at->io_mutex, timeoutms);
            timeoutms = task_getremainingtimeoutms();
            if (r == UBIK_ERR__TIMEOUT)
            {
                esp_err = ESP8266AT_ERR_TIMEOUT;
                break;
            }
            assert(r == 0);
        }
        else
        {
            r = mutex_lock(esp8266at->io_mutex);
            assert(r == 0);
        }

        written_tmp = 0;

        ubi_err = cbuf_write(esp8266at->io_write_buf, buffer, length, &written_tmp);
        assert(ubi_err == UBI_ERR_OK || ubi_err == UBI_ERR_BUF_FULL);
        if (ubi_err == UBI_ERR_BUF_FULL)
        {
            esp_err = ESP8266AT_ERR_IO_ERROR;
        }
        else
        {
            esp_err = ESP8266AT_ERR_OK;
        }

        if (written_tmp > 0)
        {
            if (!esp8266at->tx_busy)
            {
                buf = cbuf_get_head_addr(esp8266at->io_write_buf);
                len = 1;
                esp8266at->tx_busy = 1;
                for (uint32_t i = 0;; i++)
                {
                    if (HAL_UART_Transmit_IT(&ESP8266_UART_HANDLE, buf, len) == HAL_OK)
                    {
                        break;
                    }
                    if (i >= 99)
                    {
                        esp_err = ESP8266AT_ERR_IO_ERROR;
                        break;
                    }
                }
            }
        }

        if (written)
        {
            *written = written_tmp;
        }

        if ((io_option & ESP8266AT_IO_OPTION__TIMED) != 0)
        {
            if (remain_timeoutms)
            {
                *remain_timeoutms = timeoutms;
            }
        }

        r = mutex_unlock(esp8266at->io_mutex);
        assert(r == 0);
    } while (0);

    return esp_err;
}

#endif /* (UBINOS__BSP__STM32_STM32XXXX == 1) */
#endif /* (INCLUDE__ESP8266AT == 1) */

