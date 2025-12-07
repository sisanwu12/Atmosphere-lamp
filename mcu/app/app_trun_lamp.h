/**
 * @file		app_trun_lamp.h
 * @brief		用于定义抽象该模块的结构体以及声明操作该模块的函数
 * @note		基础转向灯模块
 * @author	王广平
 * @date		2025/11/23
 **/

#ifndef __APP_TRUN_LAMP_H
#define __APP_TRUN_LAMP_H

/* 头文件引用 */
#include "ERR.h"

/* 宏定义 */

#ifdef __APP_TRUN_LAMP_C /* 用于.c文件的宏 */
// clang-format off

#define LEFT_PIN			GPIO_PIN_2	/* 左转灯GPIO引脚 */
#define RIGHT_PIN			GPIO_PIN_1	/* 右转灯GPIO引脚 */
#define LEFT_GPIOx		GPIOA				/* 左转灯GPIO分组 */
#define RIGHT_GPIOx		GPIOA				/* 右转灯GPIO分组 */

// clang-format on
#endif

/* 函数声明 */
RESULT_Init app_trunL_init();
RESULT_RUN app_trunL_open_left();
RESULT_RUN app_trunL_open_right();
RESULT_RUN app_trunL_close_left();
RESULT_RUN app_trunL_close_right();

#endif