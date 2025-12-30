/**
 * @file		app_dot_displayer.h
 * @brief		定义抽象该模块的结构体以及声明操作该模块的函数。
 * @author	王广平
 */

<<<<<<< HEAD
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
/**
 * @brief 图案顺时针旋转次数（用于适配点阵模块实际安装方向）
 * @note 取值建议为 0~3；0 表示不旋转
 */
#define TurnCount 0

// clang-format on
#endif

// clang-format off

/* 空图案 */
static const u8 APP_DOTD_NULL[8] =
{
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000
};

/* 箭头图案 */
static const u8 APP_DOTD_ARROW[8] =
{
0b00011000,
0b00111100,
0b01100110,
0b11000011,
0b00011000,
0b00111100,
0b01100110,
0b11000011
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
=======
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
#define TurnCount 0           /* 转动次数 */

// clang-format on
#endif

// clang-format off

/* 空图案 */
static u8 APP_DOTD_NULL[8] = 
{
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000
};

/* 箭头图案 */
/**
 * @brief 左转箭头图案（语义：向左）
 * @note
 * bit7 通常对应最左侧 LED，bit0 对应最右侧 LED。
 * 若你的点阵模块左右相反，可在 app_dot_displayer.c 中对数据做 bit 反转，
 * 或者重新调整下列图案的位序。
 */
static u8 APP_DOTD_LEFT_ARROW[8] =
{
0b00011000,
0b00011100,
0b11111110,
0b11111111,
0b11111110,
0b00011100,
>>>>>>> 1f20ba5 (完善转向代码)
0b00011000,
0b00000000
};

<<<<<<< HEAD
// clang-format on

/* 函数声明 */
=======
/**
 * @brief 右转箭头图案（语义：向右）
 */
static u8 APP_DOTD_RIGHT_ARROW[8] =
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
static u8 APP_DOTD_START[8] =
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
static u8 APP_DOTD_UP[8] = 
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

/* 函数声明 */
>>>>>>> 1f20ba5 (完善转向代码)

/**
 * @brief 点阵显示器初始化函数
 *
 * @return RESULT_Init 初始化结果
 * @date 	2025/12/7
 */
RESULT_Init app_dotD_Init();

/**
 * @brief 按行写入点阵函数
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
RESULT_RUN app_dotD_WriteLine(u8 addr, u8 data);

/**
 * @brief 整体写入点阵函数
 *
 * @param arr 点阵对应行的数据
 * @note
 * arr[0] 写入第 1 行（addr=1），arr[7] 写入第 8 行（addr=8）
 *
 * @return RESULT_RUN 运行结果
 * @date 2025/12/11
 */
RESULT_RUN app_dotD_WriteALL(const u8 arr[8]);

/**
 * @brief 清空点阵屏的内容
 *
 * @return RESULT_RUN 运行结果
 * @date 2025/12/8
 */
RESULT_RUN app_dotD_Clear();

RESULT_RUN app_dotD_Show_LEFT();
RESULT_RUN app_dotD_Show_RIGHT();
RESULT_RUN app_dotD_Show_UP();
RESULT_RUN app_dotD_Show_START();

/**
<<<<<<< HEAD
 * @brief 转动图案函数（顺时针）
 *
 * @param old 原图案
 * @param ret 转动后的图案（输出）
 * @return 是否转动成功
 */
RESULT_RUN app_dotD_TurnWrite(const u8 old[8], u8 ret[8]);

/* 处理线程函数 */
void app_dotD_dispose_Task();

=======
 * @brief 转动图案函数（顺时针）
 *
 * @param old 原图案
 * @param ret 转动后的图案
 * @return 是否转动成功
 */
RESULT_RUN app_dotD_TurnWrite(u8 old[8], u8 ret[8]);

/* 处理线程函数 */
void app_dotD_dispose_Task();

>>>>>>> 1f20ba5 (完善转向代码)
#endif
