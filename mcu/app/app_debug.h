/**
 * @file		app_debug.h
 * @brief		定义抽象该模块的结构体以及声明操作该模块的函数。
 * @author	王广平
 */
#ifndef __APP_DEBUG_H
#define __APP_DEBUG_H

/* 引用头文件 */
#include "ERR.h"

#ifdef APP_DEBUG_C
// clang-format off

#define Debug_GPIOx		GPIOA				/* 指定GPIO分组 */
#define Debug_TX			GPIO_PIN_9	/* 指定TX引脚 */
#define Debug_RX			GPIO_PIN_10	/* 指定RX引脚 */
#define Debug_USARTx	USART1			/* 指定串口通道 */
#define Debug_BRate		115200			/* 指定串口波特率 */

// clang-format on
#endif

/* 函数声明 */
RESULT_Init app_debug_init();
void ERR_ShowBy_USART_RUN(RESULT_RUN res_run);
void ERR_ShowBy_USART_Init(RESULT_Init res_init);
void ERR_ShowBy_RGB_Init(RESULT_Init res_init);

#endif