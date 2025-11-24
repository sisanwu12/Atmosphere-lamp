/**
 * @file		bsp_timer.c
 * @brief		用于定义该模块的函数
 * @author	王广平
 * @date		2025/11/23
 */

/* 头文件引用 */
#include "ERR.h"
#include "__port_type__.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_rcc_ex.h"
#include "stm32f1xx_hal_tim.h"

/* Timer初始化函数 */

RESULT_Init bsp_timer_init(TIM_HandleTypeDef *htimx, TIM_TypeDef *timx,
                           u32 Prescaler, u32 CounterMode, u32 Period,
                           u32 ClockDivision, u32 AutoReloadPreload,
                           u32 RepetitionCounter)
{
  RESULT_Init ret = ERR_Init_Start;
  if (timx == TIM1)
    __HAL_RCC_TIM1_CLK_ENABLE();
  else if (timx == TIM2)
    __HAL_RCC_TIM2_CLK_ENABLE();
  else if (timx == TIM3)
    __HAL_RCC_TIM3_CLK_ENABLE();
  else if (timx == TIM4)
    __HAL_RCC_TIM4_CLK_ENABLE();
  htimx->Instance = timx;

  htimx->Init.Prescaler = Prescaler;
  htimx->Init.CounterMode = CounterMode;
  htimx->Init.Period = Period;
  htimx->Init.ClockDivision = ClockDivision;
  htimx->Init.AutoReloadPreload = AutoReloadPreload;
  htimx->Init.RepetitionCounter = RepetitionCounter;

  ret = ERR_Init_Finished;
  return ret;
}