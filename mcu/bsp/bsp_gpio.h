/**
 * @file		bsp_gpio.h
 * @brief		定义抽象该模块的结构体以及声明操作该模块的函数
 * @author	王广平
 * @date		2025/11/23
 */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "ERR.h"
#include "__port_type__.h"
#include "stm32f103xb.h"

/* 函数声明 */
RESULT_Init bsp_gpio_Init(GPIO_TypeDef *GPIOx, u32 GPIOpin, u32 GPIOMode,
                          u32 GPIOPull, u32 GPIOSpeed);
RESULT_Init bsp_gpio_AFPP_Init(GPIO_TypeDef *GPIOx, u32 GPIOpin);
RESULT_Init bsp_gpio_OTPP_Init(GPIO_TypeDef *GPIOx, u32 GPIOpin);
#endif