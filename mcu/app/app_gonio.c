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
#include "bsp_dma.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"

/* 静态全局变量 */
static TIM_HandleTypeDef APP_GONIO_TIM = {0};

static volatile u32 riseTime = 0;
static volatile u32 fallTime = 0;
static volatile u32 pulseWidth = 0; // us
static volatile u32 newData = 0;    // 标记是否有新数据

/**
 * @brief		角度测量模块初始化函数
 * @return	初始化结果
 * @date		2025/11/24
 **/
RESULT_Init app_gonio_init()
{
  RESULT_Init ret = ERR_Init_Start;

  /* 初始化GPIO引脚 */
  bsp_gpio_Init(GONIO_GPIOx, GONIO_PIN, GPIO_MODE_AF_INPUT, GPIO_NOPULL,
                GPIO_SPEED_FREQ_HIGH);

  /* 初始化TIM定时器 */
  bsp_timer_SetStruct(&APP_GONIO_TIM, GONIO_TIMx, 72 - 1, TIM_COUNTERMODE_UP,
                      0xFFFF, TIM_CLOCKDIVISION_DIV1,
                      TIM_AUTORELOAD_PRELOAD_DISABLE, 0);

  HAL_TIM_PWM_Init(&APP_GONIO_TIM);

  // 输入捕获配置
  TIM_IC_InitTypeDef sConfig = {0};
  // CH1: 上升沿捕获
  sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfig.ICPrescaler = TIM_ICPSC_DIV1;
  sConfig.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfig, TIM_CHANNEL_1);

  // CH2: 下降沿捕获
  sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfig, TIM_CHANNEL_2);

  HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  HAL_TIM_IC_Start_IT(&APP_GONIO_TIM, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&APP_GONIO_TIM, TIM_CHANNEL_2);

  ret = ERR_Init_Finished;
  return ret;
}

float app_gonio_GetAngleDeg(void)
{
  if (!newData)
    return -1; // 没有新数据
  newData = 0;

  const float period_us = 2000.0f; // AS5048A PWM 周期固定 2ms

  float angle = ((float)pulseWidth / period_us) * 360.0f;

  // AS5048A 12bit 0–4095，范围必须在 0–360
  if (angle < 0)
    angle = 0;
  if (angle > 360)
    angle = 360;

  return angle;
}

void TIM3_IRQHandler(void) { HAL_TIM_IRQHandler(&APP_GONIO_TIM); }

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM3)
    return;
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    // 上升沿：记录开始时间
    riseTime = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
  }
  else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    // 下降沿：记录结束时间
    fallTime = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

    if (fallTime >= riseTime)
      pulseWidth = fallTime - riseTime;
    else
      pulseWidth = (0xFFFF - riseTime) + fallTime;

    newData = 1;
  }
}