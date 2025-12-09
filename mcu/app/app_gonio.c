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
#include "FreeRTOS.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"
#include "event_bus.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "task.h"
#include <stdio.h>

/* 静态全局变量 */
/* 定时器句柄 */
static TIM_HandleTypeDef APP_GONIO_TIM = {0};

/* 方向盘初始位置 */
static float inital_value = 180;
/* us */
static volatile u32 pulseWidth = 0;
/* 是否有新数据标记 */
static volatile oboolean_t newData = bFALSE;

[[nodiscard]] RESULT_Init app_gonio_init()
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
  newData = bFALSE;

  const float period_us = 2000.0f; // AS5048A PWM 周期固定 2ms

  float angle = ((float)pulseWidth / period_us) * 360.0f;

  // AS5048A 12bit 0–4095，范围必须在 0–360
  if (angle < 0)
    angle = 0;
  if (angle > 360)
    angle = 360;

  return angle - inital_value;
}

void app_gonio_dispose_ISP()
{
  static volatile u32 riseTime = 0;
  static volatile u32 fallTime = 0;
  if (APP_GONIO_TIM.Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    // 上升沿：记录开始时间
    riseTime = HAL_TIM_ReadCapturedValue(&APP_GONIO_TIM, TIM_CHANNEL_1);
  }
  else if (APP_GONIO_TIM.Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    // 下降沿：记录结束时间
    fallTime = HAL_TIM_ReadCapturedValue(&APP_GONIO_TIM, TIM_CHANNEL_2);

    if (fallTime >= riseTime)
      pulseWidth = fallTime - riseTime;
    else
      pulseWidth = (0xFFFF - riseTime) + fallTime;

    newData = bTRUE;
  }
}

void app_gonio_dispose_Task()
{
  /* 获取方向盘旋转角度 */
  float angle = app_gonio_GetAngleDeg();

  if (angle >= 90) /* 左转 */
  {
    /* 一秒内采样100次 */
    for (u8 i = 0; i < 100; i++)
    {
      angle = app_gonio_GetAngleDeg();
      if (angle < 90) /* 有一次采样不对则返回 */
	return;
      vTaskDelay(10);
    }
    /* 认为是左转动作 */
    /* 发送左转事件 */
    xEventGroupSetBits(event_bus_getHandle(), EVT_TURN_LEFT);
  }
  else if (angle <= -90) /* 右转 */
  {
    /* 一秒内采样100次 */
    for (u8 i = 0; i < 100; i++)
    {
      angle = app_gonio_GetAngleDeg();
      if (angle > -90) /* 有一次采样不对则返回 */
	return;
      vTaskDelay(10);
    }
    /* 认为是右转动作 */
    /* 发送右转事件 */
    xEventGroupSetBits(event_bus_getHandle(), EVT_TURN_RIGHT);
  }
  else if (angle <= 30 && angle >= -30) /* 回正 */
  {
    /* 一秒内采样100次 */
    for (u8 i = 0; i < 100; i++)
    {
      angle = app_gonio_GetAngleDeg();
      if (angle > 30 || angle < -30) /* 有一次采样不对则返回 */
	return;
      vTaskDelay(10);
    }
    /* 认为是回正事件 */
    /* 发送回正消息 */
    xEventGroupSetBits(event_bus_getHandle(), EVT_TURN_BACK);
  }
}

TIM_HandleTypeDef *app_gonio_getTIMHandle() { return &APP_GONIO_TIM; }