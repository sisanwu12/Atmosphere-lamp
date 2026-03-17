/**
 * @file bsp_max7219.h
 * @brief MAX7219 board support driver.
 */

#ifndef __BSP_MAX7219_H
#define __BSP_MAX7219_H

#include "ERR.h"
#include "__port_type__.h"
#include "stm32f103xb.h"
#include <stdbool.h>

#define BSP_MAX7219_GPIOx GPIOA
#define BSP_MAX7219_DIN_PIN GPIO_PIN_7
#define BSP_MAX7219_CLK_PIN GPIO_PIN_5
#define BSP_MAX7219_CS_PIN GPIO_PIN_4
#define BSP_MAX7219_SPI SPI1

RESULT_Init bsp_max7219_init(void);
RESULT_RUN bsp_max7219_write_register(u8 addr, u8 data);
RESULT_RUN bsp_max7219_write_rows(const u8 rows[8]);
RESULT_RUN bsp_max7219_clear(void);
RESULT_RUN bsp_max7219_set_test_mode(bool enable);

#endif
