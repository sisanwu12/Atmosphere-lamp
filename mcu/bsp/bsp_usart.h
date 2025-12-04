/**
 * @file    bsp_usart.h
 * @brief   定义抽象该模块的结构体以及声明操作该模块的函数
 * @author  王广平
 */

#ifndef __BSP_USART_H
#define __BSP_USART_H

/* 引用头文件 */
#include "ERR.h"
#include "__port_type__.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_uart.h"

/* 函数声明 */
RESULT_Init bsp_usart_init(UART_HandleTypeDef *huart, USART_TypeDef *USARTx,
                           u32 BaudRate);

RESULT_RUN bsp_usart_SendStr(UART_HandleTypeDef *huart, char *str);

#endif