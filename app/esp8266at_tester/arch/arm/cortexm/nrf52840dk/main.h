#ifndef __MAIN_H
#define __MAIN_H

#include "bsp.h"
#include "nrf.h"
#include "nrf_drv_uart.h"
#include "nrf_drv_gpiote.h"

#include <esp8266at.h>

extern esp8266at_t _g_esp8266at;

/* Definition for ESP8266_UART (mikroBUS 1) */

#define ESP8266_UART_TX_Pin ARDUINO_1_PIN
#define ESP8266_UART_RX_Pin ARDUINO_0_PIN
#if (ESP8266AT__USE_RESET_PIN == 1)
    #define ESP8266_NRST_Pin ARDUINO_A3_PIN
#endif /* (ESP8266AT__USE_RESET_PIN == 1) */
#if (ESP8266AT__USE_CHIPSELECT_PIN == 1)
    #define ESP8266_CS_Pin ARDUINO_10_PIN
#endif /* (ESP8266AT__USE_CHIPSELECT_PIN == 1) */

#endif /* __MAIN_H */

