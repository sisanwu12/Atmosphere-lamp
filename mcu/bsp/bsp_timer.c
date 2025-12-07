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

/**
 * @brief   PWM模式定时器初始化函数
 * @param   htim    传入定时器结构体
 * @param   OCMode  指定输出比较模式
 * @note
 * TIM_OCMODE_PWM1：计数器 < CCR 时输出有效电平
 * TIM_OCMODE_PWM2：计数器 < CCR 时输出无效电平
 *
 * @param   Pulse   指定脉冲宽度（占空比）
 * @note
 * 实际写入的是CCRx的值。
 * 周期由 ARR 定义，占空比 = Pulse / (ARR + 1)。
 *
 * @param   OCPolarity  指定主输出极性
 * @note
 * TIM_OCPOLARITY_HIGH：有效电平 = 高电平
 * TIM_OCPOLARITY_LOW： 有效电平 = 低电平
 *
 * @param   OCFastMode  指定是否开启快速模式
 * @note
 * TIM_OCFAST_DISABLE：禁用快速模式（常用）
 * TIM_OCFAST_ENABLE： 启用快速模式
 *
 * @param   OCIdleState 指定主输出空闲状态
 * @note
 * TIM_OCIDLESTATE_RESET：空闲时输出低电平
 * TIM_OCIDLESTATE_SET：  空闲时输出高电平
 *
 * @param   OCNIdleState  指定互补输出空闲状态
 * @note
 * 同 OCIdleState，该相是给高级定时器的互补输出配置的
 *
 * @param   OCNPolarity   指定互补输出极性
 * @note
 * 同样是给高级定时器的互补输出配置的
 *
 * @param   Channel   指定输出通道
 */
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