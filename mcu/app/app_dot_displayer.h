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
#include "stm32f103xb.h"

/* ============================== 硬件宏定义 ============================== */
#ifdef APP_DOT_DISPLAYER_C
// clang-format off

/* MAX7219 点阵：SPI1 + 软件片选 */
#define DOT_GPIOx	GPIOA
#define DIN_PIN		GPIO_PIN_7
#define CLK_PIN		GPIO_PIN_5
#define CS_PIN		GPIO_PIN_4
#define DOT_SPI		SPI1

/**
 * @brief 图案顺时针旋转次数（用于适配点阵模块实际安装方向）
 * @note 取值建议为 0~3；0 表示不旋转
 */
#define TurnCount 0

// clang-format on
#endif

/* ============================== 图案数据 ============================== */
// clang-format off

/* 左转箭头图案（语义：向左） */
static const u8 APP_DOTD_LEFT_ARROW[8] =
{
  0b00011000,
  0b00011100,
  0b11111110,
  0b11111111,
  0b11111110,
  0b00011100,
  0b00011000,
  0b00000000
};

/* 右转箭头图案（语义：向右） */
static const u8 APP_DOTD_RIGHT_ARROW[8] =
{
  0b00011000,
  0b00111000,
  0b01111111,
  0b11111111,
  0b01111111,
  0b00111000,
  0b00011000,
  0b00000000
};

/* 开始图案 */
static const u8 APP_DOTD_START[8] =
{
  0b00000000,
  0b01000010,
  0b11100111,
  0b00000000,
  0b00000000,
  0b10000001,
  0b01111110,
  0b00000000
};

/* 加速图案 */
static const u8 APP_DOTD_UP[8] =
{
  0b00011000,
  0b00111100,
  0b00111100,
  0b00011000,
  0b00000000,
  0b00011000,
  0b00011000,
  0b00000000
};

// clang-format on

/* ============================== 函数声明 ============================== */
RESULT_Init app_dotD_Init();

RESULT_RUN app_dotD_WriteLine(u8 addr, u8 data);
RESULT_RUN app_dotD_WriteALL(const u8 arr[8]);
RESULT_RUN app_dotD_Clear();

RESULT_RUN app_dotD_Show_LEFT();
RESULT_RUN app_dotD_Show_RIGHT();
RESULT_RUN app_dotD_Show_UP();
RESULT_RUN app_dotD_Show_START();

/**
 * @brief 转动图案函数（顺时针旋转 90°）
 * @param old 原图案（输入）
 * @param ret 转动后的图案（输出）
 */
RESULT_RUN app_dotD_TurnWrite(const u8 old[8], u8 ret[8]);

/* 处理线程函数 */
void app_dotD_dispose_Task();

#endif
