#include <ubinos.h>
#include <ubinos/bsp/arch.h>

#if (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NUCLEOF207ZG)
#if (STM32FOOTPAD == 1)

#include "main.h"

/**
 * @brief UART MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 *           - DMA configuration for transmission request by peripheral
 *           - NVIC configuration for DMA interrupt request enable
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (huart->Instance == ESP8266_UART)
    {
        /* Configure ESP8266 UART */

        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        ESP8266_UART_TX_GPIO_CLK_ENABLE();
        ESP8266_UART_RX_GPIO_CLK_ENABLE();
        /* Enable ESP8266_UART clock */
        ESP8266_UART_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin = ESP8266_UART_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = ESP8266_UART_TX_AF;

        HAL_GPIO_Init(ESP8266_UART_TX_GPIO_Port, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = ESP8266_UART_RX_Pin;
        GPIO_InitStruct.Alternate = ESP8266_UART_RX_AF;

        HAL_GPIO_Init(ESP8266_UART_RX_GPIO_Port, &GPIO_InitStruct);

        /*##-3- Configure the NVIC for UART ########################################*/
        /* NVIC for USART */
        HAL_NVIC_SetPriority(ESP8266_UART_IRQn, NVIC_PRIO_MIDDLE, 0);
        HAL_NVIC_EnableIRQ(ESP8266_UART_IRQn);
    }
}

/**
 * @brief UART MSP De-Initialization
 *        This function frees the hardware resources used in this example:
 *          - Disable the Peripheral's clock
 *          - Revert GPIO, DMA and NVIC configuration to their default state
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == ESP8266_UART)
    {
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
}

#endif /* (STM32FOOTPAD == 1) */
#endif /* (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NUCLEOF207ZG) */

