/**
 * @file		app_dot_displayer.h
 * @brief		定义抽象该模块的结构体以及声明操作该模块的函数。
 * @author	王广平
 */

#ifndef __APP_DOT_DISPLAYER_H
#define __APP_DOT_DISPLAYER_H

/* 头文件引用 */
#include "ERR.h"
#include "__port_type__.h"

#ifdef APP_DOT_DISPLAYER_C
// clang-format off

#define DOT_GPIOx	GPIOA
#define DIN_PIN		GPIO_PIN_7
#define CLK_PIN		GPIO_PIN_5
#define CS_PIN		GPIO_PIN_4
#define DOT_SPI		SPI1

// clang-format on
#endif

/* 函数声明 */

/**
 * @brief 点阵显示器初始化函数
 *
 * @return RESULT_Init 初始化结果
 * @date 	2025/12/7
 */
RESULT_Init app_dotD_Init();

/**
 * @brief 写点阵函数
 *
 * @param addr 指定行
 * @note
 * 有效值为1~8,指定是那一行
 *
 * @param data 指定该行的值
 * @note
 * 每一比特位对应每一个灯的亮灭
 * 比如 0b10000001 只有该行的首尾两个灯亮
 *
 * @return RESULT_RUN 运行结果
 * @date	2025/12/7
 */
RESULT_RUN app_dotD_Write(u8 addr, u8 data);

/**
 * @brief 清空点阵屏的内容
 *
 * @return RESULT_RUN 运行结果
 * @date 2025/12/8
 */
RESULT_RUN app_dotD_Clear();

#endif