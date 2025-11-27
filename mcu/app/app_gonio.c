/**
 * @file		app_gonio.c
 * @brief		用于定义操作该模块的函数
 * @note		角度测量模块
 * @author	王广平
 * @date		2025/11/24
 **/

#define __APP_GONIO_C

/* 头文件引用 */
#include "app_gonio.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"

/* 静态全局变量 */
static TIM_HandleTypeDef APP_GONIO_TIM = {0};

/**
 * @brief		角度测量模块初始化函数
 * @return	初始化结果
 * @date		2025/11/24
 **/
RESULT_Init app_gonio_init()
{
  RESULT_Init ret = ERR_Init_Start;

  /* 初始化GPIO引脚 */
  bsp_gpio_Init(GONIO_GPIOx, GONIO_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL,
                GPIO_SPEED_FREQ_HIGH);

  /* 初始化TIM定时器 */
  bsp_timer_init(&APP_GONIO_TIM, GONIO_TIMx, 72 - 1, TIM_COUNTERMODE_UP, 0xFFFF,
                 TIM_CLOCKDIVISION_DIV1, TIM_AUTORELOAD_PRELOAD_DISABLE, 0);

  TIM_IC_InitTypeDef sConfigIC = {0};

  /* 上升沿捕获周期 */
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfigIC, TIM_CHANNEL_1);

  /* 下降沿捕获高电平持续时间 */
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfigIC, TIM_CHANNEL_2);

  /* 使能交叉通道 */
  __HAL_TIM_ENABLE_DMA(&APP_GONIO_TIM, TIM_DMA_CC1);

  ret = ERR_Init_Finished;
  return ret;
}