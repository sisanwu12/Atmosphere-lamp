/**
 * @file		app_gonio.h
 * @brief		用于定义抽象该模块的结构体以及声明操作该模块的函数
 * @note		角度测量模块
 * @author	王广平
 * @date		2025/11/24
 **/

#ifndef __APP_GONIO_H
#define __APP_GONIO_H

/* 头文件引用 */
#include "ERR.h"

/* 宏定义 */
// clang-format off

# ifdef __APP_GONIO_C	/* 用于.c文件的宏 */

#define GONIO_PIN			GPIO_PIN_6		/* 指定GPIO引脚 */
#define GONIO_GPIOx		GPIOA					/* 指定GPIO分组 */
#define GONIO_TIMx		TIM3					/* 指定定时器 */

#endif
// clang-format on
/* 函数声明 */
RESULT_Init app_gonio_init();

#endif