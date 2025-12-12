/**
 * @file		bsp_timer.c
 * @brief		用于定义该模块的函数
 * @author	王广平
 */

/* 头文件引用 */
#include "ERR.h"
#include "__port_type__.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_rcc_ex.h"
#include "stm32f1xx_hal_tim.h"

/**
 * @brief		定时器时钟使能函数
 * @param		timx	指定定时器
 * @date		2025/12/4
 */
static inline void bsp_timer_rcc_enable(TIM_TypeDef *timx)
{
  switch ((u32)timx)
  {
  case (u32)TIM1:
    __HAL_RCC_TIM1_CLK_ENABLE();
    break;
  case (u32)TIM2:
    __HAL_RCC_TIM2_CLK_ENABLE();
    break;
  case (u32)TIM3:
    __HAL_RCC_TIM3_CLK_ENABLE();
    break;
  case (u32)TIM4:
    __HAL_RCC_TIM4_CLK_ENABLE();
    break;
  }
}

RESULT_Init bsp_timer_SetStruct(TIM_HandleTypeDef *htim, TIM_TypeDef *timx,
                                u32 Prescaler, u32 CounterMode, u32 Period,
                                u32 ClockDivision, u32 AutoReloadPreload,
                                u32 RepetitionCounter)
{
  RESULT_Init ret = ERR_Init_Start;
  /* 定时器时钟使能 */
  bsp_timer_rcc_enable(timx);

  htim->Instance = timx;
  htim->Init.Prescaler = Prescaler;
  htim->Init.CounterMode = CounterMode;
  htim->Init.Period = Period;
  htim->Init.ClockDivision = ClockDivision;
  htim->Init.AutoReloadPreload = AutoReloadPreload;
  if (timx == TIM1)
    htim->Init.RepetitionCounter = RepetitionCounter;
  else
    htim->Init.RepetitionCounter = 0;

  ret = ERR_Init_Finished;
  return ret;
}

RESULT_Init bsp_timer_PWM_init(TIM_HandleTypeDef *htim, u32 OCMode, u32 Pulse,
                               u32 OCPolarity, u32 OCFastMode, u32 OCIdleState,
                               u32 OCNIdleState, u32 OCNPolarity, u32 Channel)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = OCMode;
  sConfigOC.Pulse = Pulse;
  sConfigOC.OCPolarity = OCPolarity;
  sConfigOC.OCFastMode = OCFastMode;
  sConfigOC.OCIdleState = OCIdleState;
  if (htim->Instance == TIM1)
  {
    sConfigOC.OCNIdleState = OCNIdleState;
    sConfigOC.OCNPolarity = OCNPolarity;
  }

  if (HAL_TIM_PWM_Init(htim) != HAL_OK)
    return ERR_Init_ERROR_TIM;
  if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, Channel) != HAL_OK)
    return ERR_Init_ERROR_TIM;
  if (HAL_TIM_PWM_Start(htim, Channel) != HAL_OK)
    return ERR_Init_ERROR_TIM;

  return ERR_Init_Finished;
}