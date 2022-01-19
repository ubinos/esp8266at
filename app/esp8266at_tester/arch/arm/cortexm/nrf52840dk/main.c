/*
 * Copyright (c) 2021 Sung Ho Park and CSOS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ubinos.h>

#if (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NRF52840DK)
#if (UBINOS__BSP__BOARD_VARIANT == 0)

#include "main.h"

esp8266at_t _g_esp8266at;

#endif /* (UBINOS__BSP__BOARD_VARIANT == 0) */
#endif /* (UBINOS__BSP__BOARD_MODEL == UBINOS__BSP__BOARD_MODEL__NRF52840DK) */

