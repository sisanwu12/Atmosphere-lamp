/**
 * @file		app_basic_trunSL.c
 * @brief		用于定义操作该模块的函数
 * @note		基础转向灯模块
 * @author	王广平
 * @date		2025/11/23
 **/

#define __APP_BASIC_TRUNSL_C

/* 头文件引用 */
#include "app_basic_trunSL.h"
#include "bsp_gpio.h"
#include "stm32f1xx_hal_gpio.h"

/**
 * @brief		初始化基础转向灯
 * @return	初始化结果
 * @date		2025/11/24
 **/
RESULT_Init app_trunSL_init()
{
  RESULT_Init ret = ERR_Init_Start;
  /* 初始化左转灯 */
  bsp_gpio_Init(LEFT_GPIOx, LEFT_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                GPIO_SPEED_FREQ_LOW);
  /* 初始化右转灯 */
  bsp_gpio_Init(RIGHT_GPIOx, RIGHT_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                GPIO_SPEED_FREQ_LOW);
  /* 初始化关闭所有灯 */
  HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN, GPIO_PIN_RESET);
  ret = ERR_Init_Finished;
  return ret;
}

/**
 * @brief		打开左转向灯
 * @return	是否成功打开
 * @date		2025/11/23
 **/
RESULT_RUN app_trunSL_open_left()
{
  HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN, GPIO_PIN_SET);
  if (HAL_GPIO_ReadPin(LEFT_GPIOx, LEFT_PIN) != GPIO_PIN_RESET)
    return ERR_RUN_ERROR_UNST;
  return ERR_RUN_Finished;
}

/**
 * @brief		打开右转向灯
 * @return	是否成功打开
 * @date		2025/11/23
 **/
RESULT_RUN app_trunSL_open_right()
{
  HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN, GPIO_PIN_SET);
  if (HAL_GPIO_ReadPin(RIGHT_GPIOx, RIGHT_PIN) != GPIO_PIN_RESET)
    return ERR_RUN_ERROR_UNST;
  return ERR_RUN_Finished;
}