/*
 * Copyright (c) 2020 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>

#if (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NUCLEOF207ZG)
#if (UBINOS__BSP__BOARD_VARIATION__STM32FOOTPAD == 1)

#include "main.h"

#if (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL)
#if (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1)
UART_HandleTypeDef DTTY_STM32_UART_HANDLE;
#endif /* (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1) */
#endif /* (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL) */

UART_HandleTypeDef ESP8266_UART_HANDLE;
esp8266at_t _g_esp8266at;

/**
 * @brief  Tx Transfer completed callback
 * @param  huart: UART handle.
 * @note   This example shows a simple way to report end of DMA Tx transfer, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
#if (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL)
#if (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1)
    if (huart->Instance == DTTY_STM32_UART)
    {
        dtty_stm32_uart_tx_callback();
        return;
    }
#endif /* (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1) */
#endif /* (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL) */

    if (huart->Instance == ESP8266_UART)
    {
        esp8266_uart_tx_callback();
        return;
    }
}

/**
 * @brief  Rx Transfer completed callback
 * @param  huart: UART handle
 * @note   This example shows a simple way to report end of DMA Rx transfer, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL)
#if (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1)
    if (huart->Instance == DTTY_STM32_UART)
    {
        dtty_stm32_uart_rx_callback();
        return;
    }
#endif /* (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1) */
#endif /* (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL) */

    if (huart->Instance == ESP8266_UART)
    {
        esp8266_uart_rx_callback();
        return;
    }
}

/**
 * @brief  UART error callbacks
 * @param  huart: UART handle
 * @note   This example shows a simple way to report transfer error, and you can
 *         add your own implementation.
 * @retval None
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
#if (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL)
#if (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1)
    if (huart->Instance == DTTY_STM32_UART)
    {
        dtty_stm32_uart_err_callback();
        return;
    }
#endif /* (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1) */
#endif /* (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL) */

    if (huart->Instance == ESP8266_UART)
    {
        esp8266_uart_err_callback();
        return;
    }
}

void power_init() {
    GPIO_InitTypeDef GPIO_InitStruct;

	//Power Pin Init
	WIFI_PW_EN_CLK_ENABLE();
	Logic_PW_EN_CLK_ENABLE();
	RF_T_EN_CLK_ENABLE();
	RF_R_EN_CLK_ENABLE();

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStruct.Pin = WIFI_PW_EN_PIN;
	HAL_GPIO_Init(WIFI_PW_EN_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Logic_PW_EN_PIN;
	HAL_GPIO_Init(Logic_PW_EN_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RF_T_EN_PIN;
	HAL_GPIO_Init(RF_T_EN_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RF_R_EN_PIN;
	HAL_GPIO_Init(RF_R_EN_PORT, &GPIO_InitStruct);
}

void wifi_enable() {
	HAL_GPIO_WritePin(WIFI_PW_EN_PORT, WIFI_PW_EN_PIN, GPIO_PIN_RESET); //WIFI pin init 0
	task_sleepms(1);
	HAL_GPIO_WritePin(WIFI_PW_EN_PORT, WIFI_PW_EN_PIN, GPIO_PIN_SET);
}


void logic_enable() {
	HAL_GPIO_WritePin(Logic_PW_EN_PORT, Logic_PW_EN_PIN, GPIO_PIN_RESET); //WIFI pin init 0
	task_sleepms(1);
	HAL_GPIO_WritePin(Logic_PW_EN_PORT, Logic_PW_EN_PIN, GPIO_PIN_SET);
}

void rf_t_enable() {
	HAL_GPIO_WritePin(RF_T_EN_PORT, RF_T_EN_PIN, GPIO_PIN_RESET); //WIFI pin init 0
	task_sleepms(1);
	HAL_GPIO_WritePin(RF_T_EN_PORT, RF_T_EN_PIN, GPIO_PIN_SET);
}

void rf_r_enable() {
	HAL_GPIO_WritePin(RF_R_EN_PORT, RF_R_EN_PIN, GPIO_PIN_RESET); //WIFI pin init 0
	task_sleepms(1);
	HAL_GPIO_WritePin(RF_R_EN_PORT, RF_R_EN_PIN, GPIO_PIN_SET);
}

#endif /* (UBINOS__BSP__BOARD_VARIATION__STM32FOOTPAD == 1) */
#endif /* (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NUCLEOF207ZG) */

