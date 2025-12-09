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
#include "__port_type__.h"
#include "stm32f1xx_hal_tim.h"

/* 宏定义 */
// clang-format off

# ifdef __APP_GONIO_C	/* 用于.c文件的宏 */

#define GONIO_PIN			GPIO_PIN_6		/* 指定GPIO引脚 */
#define GONIO_GPIOx		GPIOA					/* 指定GPIO分组 */
#define GONIO_TIMx		TIM3					/* 指定定时器 */

#endif
// clang-format on
/* 函数声明 */

/**
 * @brief		角度测量模块初始化函数
 * @return	初始化结果
 * @date		2025/11/24
 **/
RESULT_Init app_gonio_init();

/**
 * @brief 获取当前角度函数
 *
 * @return 当前角度值
 * @date 2025/12/9
 */
float app_gonio_GetAngleDeg(void);

/**
 * @brief 中断处理函数
 * @date  2025/12/9
 */
void app_gonio_dispose_ISP();

/**
 * @brief 线程处理函数
 * @date  2025/12/9
 */
void app_gonio_dispose_Task();

/**
 * @brief 获取定时器句柄
 *
 * @return 磁编码器所用定时器的句柄
 * @date 2025/12/9
 */
TIM_HandleTypeDef *app_gonio_getTIMHandle();

#endif