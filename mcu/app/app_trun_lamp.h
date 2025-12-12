/**
 * @file		app_trun_lamp.h
 * @brief		用于定义抽象该模块的结构体以及声明操作该模块的函数
 * @note		基础转向灯模块
 * @author	王广平
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

/**
 * @brief		初始化基础转向灯
 * @return	初始化结果
 * @date		2025/12/7
 **/
RESULT_Init app_trunL_init();

/**
 * @brief		打开左转向灯
 * @return	是否成功打开
 * @date		2025/12/7
 **/
RESULT_RUN app_trunL_open_left();

/**
 * @brief		打开右转向灯
 * @return	是否成功打开
 * @date		2025/12/7
 **/
RESULT_RUN app_trunL_open_right();

/**
 * @brief		关闭左转向灯
 * @return	是否成功关闭
 * @date		2025/12/7
 **/
RESULT_RUN app_trunL_close_left();

/**
 * @brief		关闭右转向灯
 * @return	是否成功关闭
 * @date		2025/12/7
 **/
RESULT_RUN app_trunL_close_right();

/**
 * @brief 处理线程函数
 * @date  2025/12/9
 */
void app_trunL_dispose_Task();

#endif