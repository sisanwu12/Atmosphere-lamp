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

/**
 * @brief   debug初始化函数
 *
 * @return  初始化结果
 * @date    2025/12/4
 */
RESULT_Init app_debug_init();

/**
 * @brief   调用USART发送运行错误信息函数，本质就是bsp_usart_SendStr函数
 * @param   usart		指定发送的通道
 * @param		res_run	指定发送的运行错误信息
 * @author	王广平
 * @date	2025/12/4
 */
void ERR_ShowBy_USART_RUN(RESULT_RUN res_run);

/**
 * @brief   调用USART发送错误初始化信息函数，本质就是bsp_usart_SendStr函数
 * @param		res_Init	指定发送的初始化错误信息
 * @author	王广平
 * @date	2025/12/4
 */
void ERR_ShowBy_USART_Init(RESULT_Init res_init);

#endif