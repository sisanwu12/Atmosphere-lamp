/**
 * @file		app_basic_trunSL.h
 * @brief		用于定义抽象该模块的结构体以及声明操作该模块的函数
 * @note		基础转向灯模块
 * @author	王广平
 * @date		2025/11/23
 **/

#ifndef __APP_BASIC_TRUNSL_H
#define __APP_BASIC_TRUNSL_H

/* 头文件引用 */
#include "ERR.h"

/* 宏定义 */
// clang-format off

# ifdef __APP_BASIC_TRUNSL_C	/* 用于.c文件的宏 */
#define LEFT_PIN			GPIO_PIN_1	/* 左转灯GPIO引脚 */
#define RIGHT_PIN			GPIO_PIN_2	/* 右转灯GPIO引脚 */
#define LEFT_GPIOx		GPIOA				/* 左转灯GPIO分组 */
#define RIGHT_GPIOx		GPIOA				/* 右转灯GPIO分组 */
#endif
// clang-format on
/* 函数声明 */
RESULT_Init app_trunSL_init();
RESULT_RUN app_trunSL_open_left();
RESULT_RUN app_trunSL_open_right();

#endif