#include <ubinos.h>

#if (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NUCLEOF207ZG)
#if (STM32FOOTPAD == 1)

#include "main.h"
#include "stm32f2xx_it.h"

/**
 * @brief  This function handles ESP8266_UART interrupt request.
 * @param  None
 * @retval None
 */
void ESP8266_UART_IRQHandler(void)
{
    HAL_UART_IRQHandler(&ESP8266_UART_HANDLE);
}

#endif /* (STM32FOOTPAD == 1) */
#endif /* (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NUCLEOF207ZG) */

