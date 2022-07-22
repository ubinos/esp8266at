#ifndef __MAIN_H
#define __MAIN_H

#include "bsp.h"
#include "nrf.h"
#include "nrf_drv_uart.h"
#include "nrf_drv_gpiote.h"

#include <esp8266at.h>

extern esp8266at_t _g_esp8266at;

/* Definition for ESP8266_UART (mikroBUS 1) */

#define ESP8266_UART_TX_Pin NRF_GPIO_PIN_MAP(0,17)
#define ESP8266_UART_RX_Pin NRF_GPIO_PIN_MAP(0,16)
#if (ESP8266AT__USE_RESET_PIN == 1)
    #define ESP8266_NRST_Pin NRF_GPIO_PIN_MAP(1,0)
#endif /* (ESP8266AT__USE_RESET_PIN == 1) */
#if (ESP8266AT__USE_CHIPSELECT_PIN == 1)
    #define ESP8266_CS_Pin NRF_GPIO_PIN_MAP(1,1)
#endif /* (ESP8266AT__USE_CHIPSELECT_PIN == 1) */
#if (ESP8266AT__USE_UART_HW_FLOW_CONTROL == 1)
    #define ESP8266_CTX_Pin NRF_GPIO_PIN_MAP(0,19)
    #define ESP8266_RTS_Pin NRF_GPIO_PIN_MAP(0,20)
#endif /* (ESP8266AT__USE_UART_HW_FLOW_CONTROL == 1) */

#endif /* __MAIN_H */

