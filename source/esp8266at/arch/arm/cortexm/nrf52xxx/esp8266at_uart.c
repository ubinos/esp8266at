/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if (INCLUDE__ESP8266AT == 1)
#if (UBINOS__BSP__NRF52_NRF52XXX == 1)

#if (INCLUDE__UBINOS__UBIK != 1)
	#error "ubik is necessary"
#endif

#include <ubinos/bsp/arch.h>

#include <assert.h>

#include "../../../../esp8266at_io.h"

#include "main.h"

static const char * _data_key = ESP8266AT_IO_DATA_KEY;

static nrf_drv_uart_t _g_esp8266at_uart = NRF_DRV_UART_INSTANCE(1);

static void esp8266at_uart_reset(void);

static void esp8266at_io_event_handler(nrf_drv_uart_event_t *p_event, void *p_context)
{
    int need_signal = 0;
    uint8_t *buf;
    uint32_t len;
    cbuf_pt rbuf;
    sem_pt rsem;
    cbuf_pt wbuf;
    sem_pt wsem;

    switch (p_event->type)
    {
    case NRF_DRV_UART_EVT_RX_DONE:
        len = ESP8266AT_IO_TEMP_RX_BUF_SIZE;
        buf = _g_esp8266at.io_temp_rx_buf;

        if (p_event->data.rxtx.bytes > 0)
        {
            switch (_g_esp8266at.io_rx_mode)
            {
            case ESP8266AT_IO_RX_MODE_RESP:
                rbuf = _g_esp8266at.io_read_buf;
                rsem = _g_esp8266at.io_read_sem;

                if (_data_key[_g_esp8266at.io_data_key_i] == buf[0])
                {
                    _g_esp8266at.io_data_key_i++;
                }
                else
                {
                    _g_esp8266at.io_data_key_i = 0;
                }
                if (_g_esp8266at.io_data_key_i == ESP8266AT_IO_DATA_KEY_LEN)
                {
                    _g_esp8266at.io_data_len = 0;
                    _g_esp8266at.io_data_len_i = 0;
                    _g_esp8266at.io_rx_mode = ESP8266AT_IO_RX_MODE_DATA_LEN;
                    break;
                }

                if (cbuf_is_full(rbuf))
                {
                    _g_esp8266at.rx_overflow_count++;
                    break;
                }

                if (cbuf_get_len(rbuf) == 0)
                {
                    need_signal = 1;
                }

                cbuf_write(rbuf, buf, len, NULL);

                if (need_signal && _bsp_kernel_active)
                {
                    sem_give(rsem);
                }

                break;

            case ESP8266AT_IO_RX_MODE_DATA_LEN:
                rbuf = _g_esp8266at.io_read_buf;
                rsem = _g_esp8266at.io_read_sem;

                if (':' == buf[0])
                {
                    _g_esp8266at.io_data_len_buf[_g_esp8266at.io_data_len_i] = 0;
                    _g_esp8266at.io_data_len = atoi((char*) _g_esp8266at.io_data_len_buf);
                    if (_g_esp8266at.io_data_len > ESP8266AT_IO_DATA_LEN_MAX)
                    {
                        _g_esp8266at.io_data_key_i = 0;
                        _g_esp8266at.io_rx_mode = ESP8266AT_IO_RX_MODE_RESP;
                        break;
                    }

                    _g_esp8266at.io_data_read = 0;
                    _g_esp8266at.io_rx_mode = ESP8266AT_IO_RX_MODE_DATA;
                    break;
                }

                if (_g_esp8266at.io_data_len_i >= ESP8266AT_IO_DATA_LEN_BUF_SIZE - 1)
                {
                    _g_esp8266at.io_data_key_i = 0;
                    _g_esp8266at.io_rx_mode = ESP8266AT_IO_RX_MODE_RESP;
                    break;
                }

                _g_esp8266at.io_data_len_buf[_g_esp8266at.io_data_len_i] = buf[0];
                _g_esp8266at.io_data_len_i++;

                break;

            case ESP8266AT_IO_RX_MODE_DATA:
                rbuf = _g_esp8266at.io_data_buf;
                rsem = _g_esp8266at.io_data_read_sem;

                _g_esp8266at.io_data_read += len;

                if (cbuf_is_full(rbuf))
                {
                    _g_esp8266at.rx_overflow_count++;

                    if (_g_esp8266at.io_data_read >= _g_esp8266at.io_data_len)
                    {
                        _g_esp8266at.io_data_key_i = 0;
                        _g_esp8266at.io_rx_mode = ESP8266AT_IO_RX_MODE_RESP;
                    }
                    break;
                }

                if (cbuf_get_len(rbuf) == 0)
                {
                    need_signal = 1;
                }

                cbuf_write(rbuf, buf, len, NULL);

                if (need_signal && _bsp_kernel_active)
                {
                    sem_give(rsem);
                }
                if (_g_esp8266at.io_data_read >= _g_esp8266at.io_data_len)
                {
                    _g_esp8266at.io_data_key_i = 0;
                    _g_esp8266at.io_rx_mode = ESP8266AT_IO_RX_MODE_RESP;
                }

                break;
            }
        }

        nrf_drv_uart_rx(&_g_esp8266at_uart, buf, len);

        break;

    case NRF_DRV_UART_EVT_TX_DONE:
        wbuf = _g_esp8266at.io_write_buf;
        wsem = _g_esp8266at.io_write_sem;

        if (p_event->data.rxtx.bytes > 0)
        {
            cbuf_read(wbuf, NULL, p_event->data.rxtx.bytes, NULL);
        }

        if (cbuf_get_len(wbuf) > 0)
        {
            buf = cbuf_get_head_addr(wbuf);
            len = 1;
            nrf_drv_uart_tx(&_g_esp8266at_uart, buf, len);
        }
        else
        {
            _g_esp8266at.tx_busy = 0;
            sem_give(wsem);
        }

        break;

    case NRF_DRV_UART_EVT_ERROR:
        break;

    default:
        break;
    }
}

static void esp8266at_uart_reset(void)
{
    ret_code_t nrf_err;
    nrf_drv_uart_config_t config;

    config.pseltxd = ARDUINO_1_PIN;
    config.pselrxd = ARDUINO_0_PIN;
    config.pselcts = CTS_PIN_NUMBER;
    config.pselrts = RTS_PIN_NUMBER;
    config.p_context = NULL;
    config.hwfc = NRF_UART_HWFC_DISABLED;
    config.parity = NRF_UART_PARITY_EXCLUDED;
    config.baudrate = NRF_UART_BAUDRATE_115200;
    config.interrupt_priority = NVIC_PRIO_LOWEST;
#if defined(NRF_DRV_UART_WITH_UARTE) && defined(NRF_DRV_UART_WITH_UART)
    config.use_easy_dma = true;
#endif
    nrf_err = nrf_drv_uart_init(&_g_esp8266at_uart, &config, esp8266at_io_event_handler);
    assert(nrf_err == NRF_SUCCESS);
}

esp8266at_err_t esp8266at_io_init(esp8266at_t *esp8266at)
{
    esp8266at_err_t esp_err;
    assert(esp8266at != NULL);

    esp8266at_uart_reset();

    if (!cbuf_is_full(esp8266at->io_read_buf))
    {
        nrf_drv_uart_rx(&_g_esp8266at_uart, esp8266at->io_temp_rx_buf, ESP8266AT_IO_TEMP_RX_BUF_SIZE);
    }

    esp_err = ESP8266AT_ERR_OK;

    return esp_err;
}

esp8266at_err_t esp8266at_io_deinit(esp8266at_t *esp8266at)
{
    esp8266at_err_t esp_err;
    assert(esp8266at != NULL);

    nrf_drv_uart_uninit(&_g_esp8266at_uart);

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
                    if (!nrf_drv_uart_tx_in_progress(&_g_esp8266at_uart))
                    {
                        nrf_drv_uart_tx(&_g_esp8266at_uart, buf, len);
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

#endif /* (UBINOS__BSP__NRF52_NRF52XXX == 1) */
#endif /* (INCLUDE__ESP8266AT == 1) */

