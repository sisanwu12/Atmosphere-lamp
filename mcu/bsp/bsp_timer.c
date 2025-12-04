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

/**
 * @brief 	timer定时器初始化函数
 * @param		htim
 *	传入定时器结构体指针
 * @param		timx					指定定时器
 * @param		Prescaler			指定预分频系数
 * @param		CounterMode		指定计数模式
 * @param		Period				指定自动装填值
 * @param		ClockDivision	指定时钟分频系数
 * @param		AutoReloadPreload
 *	ARR预装载,指定定时器的周期（ARR）是马上生效，还是等下一次更新事件才生效
 * @param		RepetitionCounter
 * 指定定时器更新事件的触发频率为每 n+1 次触发一次
 * @return	初始化的结果
 * @date		2025/11/24
 **/
RESULT_Init bsp_timer_init(TIM_HandleTypeDef *htim, TIM_TypeDef *timx,
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

  ret = HAL_TIM_IC_Init(htim) ? ERR_Init_ERROR_TIM : ERR_Init_Finished;
  if (ret)
    return ret;

  ret = ERR_Init_Finished;
  return ret;
}