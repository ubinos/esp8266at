/**
  ******************************************************************************
  * @file    UART/UART_Printf/Src/stm32f2xx_hal_msp.c
  * @author  MCD Application Team
  * @brief   HAL MSP module.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F2xx_HAL_Examples
  * @{
  */

/** @defgroup HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

/**
  * @brief UART MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* The ESP8266 'RST' IO must remain high during communication with WiFi module*/

  /* Enable the GPIO clock */
  ESP8266_NRST_GPIO_CLK_ENABLE();
  ESP8266_NCS_GPIO_CLK_ENABLE();

  /* Set the RST GPIO pin configuration parametres */
  GPIO_InitStruct.Pin       = ESP8266_NRST_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;

  /* Configure the RST IO */
  HAL_GPIO_Init(ESP8266_NRST_GPIO_Port, &GPIO_InitStruct);

  /* Set the CS GPIO pin configuration parametres */
  GPIO_InitStruct.Pin       = ESP8266_NCS_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;

  /* Configure the CS IO */
  HAL_GPIO_Init(ESP8266_NCS_GPIO_Port, &GPIO_InitStruct);

  /* Set the RST IO low */
  HAL_GPIO_WritePin(ESP8266_NRST_GPIO_Port, ESP8266_NRST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ESP8266_NCS_GPIO_Port, ESP8266_NCS_Pin, GPIO_PIN_SET);
  HAL_Delay(100);

  /* Set the RST IO high */
  HAL_GPIO_WritePin(ESP8266_NRST_GPIO_Port, ESP8266_NRST_Pin, GPIO_PIN_SET);

  /* Wait for the device to be ready */
  HAL_Delay(500);

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  ESP8266_UART_TX_GPIO_CLK_ENABLE();
  ESP8266_UART_RX_GPIO_CLK_ENABLE();

  /* Enable ESP8266_UART clock */
  ESP8266_UART_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = ESP8266_UART_TX_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = ESP8266_UART_TX_AF;

  HAL_GPIO_Init(ESP8266_UART_TX_GPIO_Port, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = ESP8266_UART_RX_Pin;
  GPIO_InitStruct.Alternate = ESP8266_UART_RX_AF;

  HAL_GPIO_Init(ESP8266_UART_RX_GPIO_Port, &GPIO_InitStruct);

  /*##-3- Configure the NVIC for UART ########################################*/
  /* NVIC for USART */
  HAL_NVIC_SetPriority(ESP8266_UART_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(ESP8266_UART_IRQn);

  //
  DTTY_STM32_UART_TX_GPIO_CLK_ENABLE();
  DTTY_STM32_UART_RX_GPIO_CLK_ENABLE();

  DTTY_STM32_UART_CLK_ENABLE();

  GPIO_InitStruct.Pin       = DTTY_STM32_UART_TX_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = DTTY_STM32_UART_TX_AF;

  HAL_GPIO_Init(DTTY_STM32_UART_TX_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DTTY_STM32_UART_RX_Pin;
  GPIO_InitStruct.Alternate = DTTY_STM32_UART_RX_AF;

  HAL_GPIO_Init(DTTY_STM32_UART_RX_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(DTTY_STM32_UART_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(DTTY_STM32_UART_IRQn);
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  DTTY_STM32_UART_FORCE_RESET();
  DTTY_STM32_UART_RELEASE_RESET();

  HAL_GPIO_DeInit(DTTY_STM32_UART_TX_GPIO_Port, DTTY_STM32_UART_TX_Pin);
  HAL_GPIO_DeInit(DTTY_STM32_UART_RX_GPIO_Port, DTTY_STM32_UART_RX_Pin);

  HAL_NVIC_DisableIRQ(DTTY_STM32_UART_IRQn);

  /*##-1- Reset peripherals ##################################################*/
  ESP8266_UART_FORCE_RESET();
  ESP8266_UART_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(ESP8266_UART_TX_GPIO_Port, ESP8266_UART_TX_Pin);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(ESP8266_UART_RX_GPIO_Port, ESP8266_UART_RX_Pin);

  /*##-3- Disable the NVIC for UART ##########################################*/
  HAL_NVIC_DisableIRQ(ESP8266_UART_IRQn);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
