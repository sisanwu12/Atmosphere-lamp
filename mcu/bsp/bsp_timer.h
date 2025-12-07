/**
 * @file		bsp_timer.h
 * @brief		定义抽象该模块的结构体以及声明操作该模块的函数
 * @author	王广平
 **/

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include "ERR.h"
#include "__port_type__.h"
#include "stm32f1xx_hal_tim.h"

RESULT_Init bsp_timer_SetStruct(TIM_HandleTypeDef *htim, TIM_TypeDef *timx,
                                u32 Prescaler, u32 CounterMode, u32 Period,
                                u32 ClockDivision, u32 AutoReloadPreload,
                                u32 RepetitionCounter);
RESULT_Init bsp_timer_PWM_init(TIM_HandleTypeDef *htim, u32 OCMode, u32 Pulse,
                               u32 OCPolarity, u32 OCFastMode, u32 OCIdleState,
                               u32 OCNIdleState, u32 OCNPolarity, u32 Channel);

#endif