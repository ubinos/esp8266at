/**
  ******************************************************************************
  * @file    UART/UART_Printf/Inc/main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_hal.h"
#include "stm32f2xx_nucleo_144.h"
#include "stdio.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */
/* Definition for USARTx clock resources */
#define ESP8266_UART_HANDLE                     huart6

#define ESP8266_UART                            USART6
#define ESP8266_UART_CLK_ENABLE()               __HAL_RCC_USART6_CLK_ENABLE();
#define ESP8266_UART_RX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOG_CLK_ENABLE()
#define ESP8266_UART_TX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOG_CLK_ENABLE()

#define ESP8266_UART_FORCE_RESET()              __HAL_RCC_USART6_FORCE_RESET()
#define ESP8266_UART_RELEASE_RESET()            __HAL_RCC_USART6_RELEASE_RESET()

/* Definition for USARTx Pins */
#define ESP8266_UART_TX_Pin                     GPIO_PIN_14
#define ESP8266_UART_TX_GPIO_Port               GPIOG
#define ESP8266_UART_TX_AF                      GPIO_AF8_USART6
#define ESP8266_UART_RX_Pin                     GPIO_PIN_9
#define ESP8266_UART_RX_GPIO_Port               GPIOG
#define ESP8266_UART_RX_AF                      GPIO_AF8_USART6

/* Definition for USARTx's NVIC IRQ and IRQ Handlers */
#define ESP8266_UART_IRQn                       USART6_IRQn
#define ESP8266_UART_IRQHandler                 USART6_IRQHandler

/* WiFi module Reset pin definitions */
#define ESP8266_NRST_GPIO_Port                  GPIOC
#define ESP8266_NRST_Pin                        GPIO_PIN_3
#define ESP8266_NRST_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOC_CLK_ENABLE()

/* WiFi module CS pin definitions */
#define ESP8266_NCS_GPIO_Port                   GPIOD
#define ESP8266_NCS_Pin                         GPIO_PIN_15
#define ESP8266_NCS_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOD_CLK_ENABLE()

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
