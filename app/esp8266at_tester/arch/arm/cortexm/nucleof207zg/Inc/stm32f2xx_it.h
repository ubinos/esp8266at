/**
 ******************************************************************************
 * @file    UART/UART_Printf/Inc/stm32f2xx_it.h
 * @author  MCD Application Team
 * @brief   This file contains the headers of the interrupt handlers.
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
#ifndef __STM32F2xx_IT_H
#define __STM32F2xx_IT_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* Interrupt Handler */
#if (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL)
#if (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1)

void DTTY_STM32_UART_IRQHandler(void);

#endif /* (STM32CUBEF2__DTTY_STM32_UART_ENABLE == 1) */
#endif /* (UBINOS__BSP__DTTY_TYPE == UBINOS__BSP__DTTY_TYPE__EXTERNAL) */

void ESP8266_UART_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F2xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
